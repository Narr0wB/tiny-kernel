
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>
#include <tiny/errno.h>

#include <tiny/fs/vfs.h>

static DEFINE_HASHTABLE(mnthash, 5);
static DEFINE_HASHTABLE(inohash, 12);

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);
static struct dentry d_root;
static struct inode  i_root;
static struct mount  mnt_root;

void init_vfs()
{
    mnt_root.mountpoint = &d_root;
    mnt_root.root = &d_root;
    mnt_root.parent = NULL;
    mnt_root.sb = NULL;

    list_head_init(&mnt_root.child);
    list_head_init(&mnt_root.sub_mnts);

    d_root.inode = &i_root;
    d_root.name = QSTR("/");
    d_root.parent = NULL;
    d_root.flags = 0;
    atomic_set(&d_root.ref, 0);

    list_head_init(&d_root.child);
    list_head_init(&d_root.subdirs);
}

void register_filesystem(struct filesystem *fs)
{
    list_add(fs, &filesystems);
}

void unregister_filesystem(const char *name)
{
    struct filesystem *fs;
    list_foreach_entry(&filesystems, fs, list) {
        if (strcmp(fs->name, name) == 0)
            list_del(&fs->list);
    }
}

void dput(struct dentry *entry)
{
    if (list_empty(&entry->subdirs))
        return;

    struct dentry *pos = NULL;
    struct dentry *before = NULL;
    list_foreach_entry(&entry->subdirs, pos, child) {
        if (before)
            kfree(before);

        dput(pos);
        before = pos;
    }

    if (before)
        kfree(before);
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

    if (fs->fill_super(sb) != 0) {
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

struct mount *mount_bdev(struct filesystem *fs, dev_t dev, struct mount *parent, struct dentry *mountpoint)
{
    struct superblock *sb = sget(fs, dev);
    if (!sb)
        return NULL;

    struct mount *mnt = kmalloc(sizeof(struct mount), PAL_KERNEL);

    list_head_init(&mnt->sub_mnts);
    mnt->root = sb->root;
    mnt->sb   = sb;

    if (graft_tree(mnt, parent, mountpoint)) {
        kfree(mnt);
        deactivate_super(sb);
        return NULL;
    }
    
    return mnt;
}

int umount(struct dentry *mountpoint)
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

struct inode *alloc_inode(struct superblock *sb)
{

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

    inode = alloc_inode(sb);
    inode->ino = ino;

    if (sb->ops->read_inode(inode)) {
        kfree(inode);
        return NULL;
    }

    hash_add(inohash, &inode->hnode, hash_inode(sb, ino));
    list_add(&inode->sb_list, &sb->inodes);
    return inode;
}

int iput(struct inode *inode)
{
    if (!inode)
        return -ENOENT;

    if (!atomic_dec_and_test(&inode->ref))
        return -EBUSY;
    
    if (inode->links == 0) {
        if (inode->ops && inode->ops->truncate)
            inode->ops->truncate(inode);
    }

    list_del(&inode->sb_list);
    hlist_del(&inode->hnode);

    kfree(inode);
    return 0;
}