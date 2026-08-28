
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>
#include <tiny/errno.h>

#include <tiny/fs/vfs.h>

static DEFINE_HASHTABLE(mnthash, 5);
static DEFINE_HASHTABLE(inohash, 12);
static DEFINE_HASHTABLE(dcache,  12);

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);

struct dentry d_root;
struct mount  mnt_root;

void init_vfs()
{
    mnt_root.mountpoint = &d_root;
    mnt_root.root = &d_root;
    mnt_root.parent = &mnt_root;
    mnt_root.sb = NULL;

    list_head_init(&mnt_root.child);
    list_head_init(&mnt_root.sub_mnts);

    d_root.name = QSTR("/");
    d_root.parent = &d_root;
    d_root.flags = 0;
    atomic_set(&d_root.ref, 0);

    list_head_init(&d_root.child);
    list_head_init(&d_root.subdirs);
}

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

inline struct superblock *alloc_super(struct filesystem *fs, dev_t dev)
{
    struct superblock *s = kmalloc(sizeof(struct superblock), PAL_KERNEL);
    if (!s)
        return NULL;

    s->ops = NULL;
    list_head_init(&s->list);
    list_head_init(&s->inodes);
    s->fs = fs;
    s->device = dev;
    s->flags = 0;
    s->root = NULL;
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

// struct inode *iget(struct superblock *sb, ino_t ino)
// {
//     struct inode *inode = NULL;
//     list_foreach_entry(&sb->inodes, inode, sb_list) {
//         if (inode->ino == ino) {
//             atomic_inc(&inode->ref);
//             return inode;
//         }
//     }

//     inode = sb->ops->alloc_inode(sb);
//     inode->ino = ino;

//     hash_add(inohash, &inode->hnode, hash_inode(sb, ino));
//     list_add(&inode->sb_list, &sb->inodes);
//     return inode;
// }

// int iput(struct inode *inode)
// {
//     if (!inode)
//         return -ENOENT;

//     if (!atomic_dec_and_test(&inode->ref))
//         return -EBUSY;
    
//     if (inode->links == 0)
//         inode->sb->ops->evict_inode(inode);

//     list_del(&inode->sb_list);
//     hlist_del(&inode->hnode);

//     kfree(inode);
//     return 0;
// }

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
    entry->name.str = kstrdup(name->str, name->len);
    entry->name.len = name->len;
    entry->flags = 0;

    if (!parent)
        entry->parent = entry;
    else
        entry->parent = dget(parent);

    hash_add(dcache, &entry->hnode, dhash(parent, name));
    list_add(&entry->child, &parent->subdirs);
    list_headinit(&entry->subdirs);

    atomic_set(&entry->ref, 1);

    return entry;
}

void dinstantiate(struct dentry *dentry, struct inode *inode)
{
    dentry->inode = inode;
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

struct dentry *dget(struct dentry *dentry)
{
    atomic_inc(&dentry->ref);
    return dentry;
}

void dput(struct dentry *dentry)
{
    if (!atomic_dec_and_test(&dentry->ref))
        return;

    ddelete(dentry);
}



int graft_tree(struct mount *mnt, struct mount *parent, struct dentry *mountpoint)
{
    if (!d_is_dir(mountpoint))
        return -ENODIR;

    mountpoint->flags |= DCACHE_MOUNTED;

    mnt->mountpoint = mountpoint;
    mnt->parent = parent;

    list_add_tail(&mnt->child, &parent->sub_mnts);
    hash_add(mnthash, &mnt->hnode, (uint64_t)mountpoint);

    return 0;
}

struct mount *vfs_mount(struct filesystem *fs, dev_t dev, struct path *path)
{
    struct superblock *sb = sget(fs, dev);
    if (!sb)
        return NULL;

    struct mount *mnt = kmalloc(sizeof(struct mount), PAL_KERNEL);

    list_head_init(&mnt->sub_mnts);
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
    if (!(mountpoint->flags & DCACHE_MOUNTED) || mountpoint == &d_root)
        return -EINVAL;
    
    struct mount *mnt = NULL;
    hlist_for_each_possible(mnthash, mnt, hnode, (uint64_t)mountpoint) {
        if (mnt->mountpoint == mountpoint) 
            break;
    }

    if (!mnt)
        return -ENOENT;

    if (!list_empty(&mnt->sub_mnts))
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

int vfs_open(const struct path *path, u32 flags, struct file **result)
{
    if (!path || !path->dentry || !path->mnt)
        return -EINVAL;

    struct inode *ino = path->dentry->inode;
    if (!ino)
        return -ENOENT;

    struct file *file = (struct file *)kmalloc(sizeof(struct file), PAL_KERNEL);

    if (!file)
        return -ENOMEM;

    file->dentry = dget(path->dentry);
    file->mount  = path->mnt;
    file->ops    = ino->fops;
    file->flags  = 0;
    file->mode   = 0;
    file->pos    = 0;

    *result = file;
    return 0;
}

int vfs_close(struct file *f)
{
    if (!f)
        return -EINVAL;

    dput(f->dentry);

    kfree(f);
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

int vfs_rmdir(struct inode *parent, struct dentry *dir)
{
    if (!dir || !parent || !dir->inode)
        return -EINVAL;

    if (!d_is_dir(dir))
        return -ENODIR;

    if (dir->flags & DCACHE_MOUNTED)
        return -EBUSY;
    
    int err = parent->ops->rmdir(parent, dir);
    if (err)
        return err;

    d_delete(dir);
    return 0;
}