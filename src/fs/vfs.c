
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>
#include <tiny/errno.h>

#include <tiny/fs/vfs.h>

static DEFINE_HASHTABLE(mnthash, 5);
static DEFINE_HASHTABLE(dcache,  12);

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);

struct dentry droot;
struct inode  iroot;

void register_filesystem(struct filesystem *fs)
{
    list_add(&fs->list, &filesystems);
}

void unregister_filesystem(const char *name)
{
    struct filesystem *fs;
    list_foreach_entry(&filesystems, fs, list) {
        if (strcmp(fs->name, name) == 0)
            list_del(&fs->list);
    }
}

struct superblock *alloc_super(struct filesystem *fs, dev_t dev)
{
    struct superblock *s = kmalloc(sizeof(struct superblock), PAL_KERNEL);
    if (!s)
        return NULL;

    s->ops = NULL;
    s->fs = fs;
    s->device = dev;
    s->flags = 0;
    s->root = NULL;

    list_head_init(&s->list);
    list_head_init(&s->inodes);

    atomic_set(&s->count, 1);

    return s;
}

struct superblock *sget(struct filesystem *fs, dev_t dev)
{
    /* If the device already has a superblock, then yield */
    struct superblock *sb = NULL;
    list_foreach_entry(&superblocks, sb, list) {
        if (dev != 0 && sb->device == dev && sb->fs == fs) {
            atomic_inc(&sb->count);
            return sb;
        }
    }

    sb = alloc_super(fs, dev);
    if (!sb)
        return NULL;

    if (fs->fill_super(sb)) {
        kfree(sb);
        return NULL;
    }

    list_add(&sb->list, &superblocks);
    return sb; 
}

int deactivate_super(struct superblock *sb)
{
    if (atomic_dec_and_test(&sb->count) > 0)
        return 0;

    if (sb->fs && sb->fs->kill_super)
        sb->fs->kill_super(sb);

    if (sb->root) {
        dput(sb->root);
        kfree(sb->root);
    }

    list_del(&sb->list);
    kfree(sb);
    return 0;
}

static __force_inline u64 hash_inode(struct superblock *sb, ino_t ino)
{
    return (uintptr_t)sb ^ ino;
}

struct inode *iget(struct superblock *sb, ino_t ino)
{
    struct inode *inode = NULL;
    list_foreach_entry(&sb->inodes, inode, sb_list) {
        if (inode->ino == ino) {
            atomic_inc(&inode->ref);
            return inode;
        }
    }

    inode = sb->ops->alloc_inode(sb);
    inode->ino = ino;

    list_add(&inode->sb_list, &sb->inodes);
    return inode;
}

void iput(struct inode *inode)
{
    if (!atomic_dec_and_test(&inode->ref))
        return;
    
    list_del(&inode->sb_list);

    kfree(inode);
}

static __force_inline u64 dhash(struct dentry *parent, struct qstr *name)
{
    u64 key = 0;
    for (int i = 0; i < name->len; ++i)
        key ^= name->str[i];
    return key ^ (uintptr_t)parent;
}

struct dentry *dalloc(struct dentry *parent, struct qstr *name)
{
    struct dentry *entry = (struct dentry *)kmalloc(sizeof(struct dentry), PAL_KERNEL);
    if (!entry)
        return NULL;

    entry->inode = NULL;
    entry->flags = 0;

    if (name) {
        entry->name.str = kstrdup(name->str, name->len);
        entry->name.len = name->len;

        /* Anonymous dentries are usually superblock/mount roots, which do not need cached */
        hash_add(dcache, &entry->hnode, dhash(parent, name));
    }

    if (parent) {
        entry->parent = dget(parent);
        list_add(&entry->child, &parent->subdirs);
    }
    else {
        entry->parent = entry;
    }

    list_head_init(&entry->subdirs);
    atomic_set(&entry->ref, 1);

    return entry;
}

struct dentry *dlookup(struct dentry *parent, struct qstr *name)
{
    /* First, check dcache */
    struct dentry *entry = NULL;
    hlist_for_each_possible(dcache, entry, hnode, dhash(parent, name)) {
        if (qstr_cmp(&entry->name, name) && entry->parent == parent)
            return dget(entry);
    }

    /* Second, check children of parent */
    list_foreach_entry(&parent->subdirs, entry, child) {
        if (qstr_cmp(&entry->name, name) && entry->parent == parent)
            return dget(entry);
    }

    /* Third, create entry in the dtree */
    entry = dalloc(parent, name);
    if (!entry || parent->inode->ops->lookup(parent->inode, entry, 0)) {
        dput(entry);
        return NULL;
    }

    return entry;
}

void ddelete(struct dentry *dentry)
{
    list_del(&dentry->child);
    hlist_del(&dentry->hnode);

    if (dentry->inode) 
        iput(dentry->inode);

    if (dentry->parent)
        dput(dentry->parent);

    kfree(dentry->name.str);
    kfree(dentry);
}



int graft_tree(struct mount *mnt, struct mount *parent, struct dentry *mountpoint)
{
    if (!d_is_dir(mountpoint))
        return -ENODIR;

    mountpoint->flags |= DCACHE_MOUNTED;
    mnt->mountpoint = mountpoint;

    if (parent) {
        mnt->parent = parent;
        list_add_tail(&mnt->child, &parent->submnts);
    }
    else {
        /* We are grafting the root mount */
        mnt->parent = mnt; 
    }

    hash_add(mnthash, &mnt->hnode, (uint64_t)mountpoint);

    return 0;
}

struct mount *vfs_mount(struct filesystem *fs, dev_t dev, struct path *path)
{
    struct superblock *sb = sget(fs, dev);
    if (!sb)
        return NULL;

    struct mount *mnt = kmalloc(sizeof(struct mount), PAL_KERNEL);

    list_head_init(&mnt->submnts);
    mnt->root = sb->root;
    mnt->sb   = sb;

    if (graft_tree(mnt, path->mnt, path->dentry)) {
        kfree(mnt);
        deactivate_super(sb);
        return NULL;
    }
    
    return mnt;
}

int vfs_umount(struct dentry *mountpoint)
{
    if (!(mountpoint->flags & DCACHE_MOUNTED) || mountpoint == &droot)
        return -EINVAL;
    
    struct mount *mnt = NULL;
    hlist_for_each_possible(mnthash, mnt, hnode, (uint64_t)mountpoint) {
        if (mnt->mountpoint == mountpoint) 
            break;
    }

    if (!mnt)
        return -ENOENT;

    if (!list_empty(&mnt->submnts))
        return -EBUSY;

    list_del(&mnt->child);
    hlist_del(&mnt->hnode);

    mountpoint->flags &= ~DCACHE_MOUNTED;

    deactivate_super(mnt->sb);
    kfree(mnt);

    return 0;
}

struct mount *vfs_lookup_mount(struct dentry *mountpoint)
{
    if (!mountpoint || !mountpoint->flags & DCACHE_MOUNTED)
        return NULL;

    struct mount *mnt = NULL; 
    hlist_for_each_possible(mnthash, mnt, hnode, (u64)mountpoint) {
        if (mnt->mountpoint == mountpoint)
            return mnt;
    }
}

int vfs_open(const struct path *path, struct file **result, u32 flags)
{
    if (!path || !path->dentry || !path->mnt)
        return -EINVAL;

    struct inode *ino = path->dentry->inode;
    if (!ino)
        return -ENOENT;

    int err = ino->fops->open(ino, result, flags);
    return 0;
}

int vfs_close(struct file *f)
{
    if (!f)
        return -EINVAL;

    f->ops->release(f);
    return 0;
}

ssize_t vfs_read(struct file *f, void *buf, size_t sz)
{
    if (!f || !f->ops || !f->ops->read)
        return -EINVAL;

    if (!sz)
        return 0;

    ssize_t read = f->ops->read(f, buf, sz);
    if (read > 0)
        f->pos += read;

    return read;
}

ssize_t vfs_write(struct file *f, const void *buf, size_t sz)
{
    if (!f || !f->ops || !f->ops->write)
        return -EINVAL;

    if (!sz)
        return 0;

    ssize_t written = f->ops->write(f, buf, sz);
    if (written > 0)
        f->pos += written;

    return written;
}

int vfs_getattr(const struct path *path, struct stat *stat)
{
    // TODO
}

int vfs_statfs(struct superblock *sb, struct statfs *stat)
{
    // TODO
}

int vfs_create(struct inode *parent, struct dentry *negative, mode_t mode)
{
    if (!parent || !parent->ops || !parent->ops->create || !negative)
        return -EINVAL;

    if (!S_ISDIR(parent->mode))
        return -ENODIR;

    int err = parent->ops->create(parent, negative, mode);
    return err;
}

int vfs_remove(struct inode *parent, struct dentry *dentry)
{
    if (!parent || !parent->ops || !parent->ops->truncate)
        return -EINVAL;

    if (!dentry || !dentry->inode || d_is_dir(dentry))
        return -EINVAL;

    if (!S_ISDIR(parent->mode))
        return -ENODIR;
    
    int err = parent->ops->truncate(dentry->inode);
    return err;
}

int vfs_mkdir(struct inode *parent, struct dentry *negative, mode_t mode)
{
    if (!parent || !parent->ops || !parent->ops->mkdir || !negative)
        return -EINVAL;

    if (!S_ISDIR(parent->mode))
        return -ENODIR;

    int err = parent->ops->mkdir(parent, negative, mode);
    return err;
}

int vfs_rmdir(struct inode *parent, struct dentry *dir)
{
    if (!dir || !parent || !dir->inode)
        return -EINVAL;

    if (!S_ISDIR(parent->mode) || !d_is_dir(dir))
        return -ENODIR;

    if (dir->flags & DCACHE_MOUNTED)
        return -EBUSY;
    
    int err = parent->ops->rmdir(parent, dir);
    if (err)
        return err;

    ddelete(dir);
    return 0;
}

void init_vfs()
{
    iroot.ino = 0;
    iroot.sb  = NULL;
    iroot.mode = S_IFDIR;
    iroot.size = 0;
    iroot.links = 1;
    iroot.ops   = NULL;
    iroot.fops  = NULL;
    atomic_set(&iroot.ref, 1);

    droot.inode = &iroot;
    droot.name = QSTR("/");
    droot.parent = &droot;
    droot.flags = 0;
    atomic_set(&droot.ref, 0);
}