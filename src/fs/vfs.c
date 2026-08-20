
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>
#include <tiny/errno.h>

#include <tiny/fs/vfs.h>

static DEFINE_HASHTABLE(mnthash, 5);

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);
static LIST_HEAD(d_root);

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
        before = pos;

        if (&pos->child != entry->subdirs.next) {
            kfree(before);
        }
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

    if (sb->root)
        dput(sb->root);

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
    if (!(mountpoint->flags & DCACHE_MOUNTED))
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