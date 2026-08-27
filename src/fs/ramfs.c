
#include <tiny/mm/kmalloc.h>

#include <tiny/fs/ramfs.h>

static u32 ramfs_monotonic_count = 0;
static struct inode_ops ramfs_iops;

static u32 ramfs_get_next_ino()
{
    return ramfs_monotonic_count++;
}

static struct inode *ramfs_get_inode(struct superblock *sb, struct inode *dir, mode_t mode, dev_t dev)
{
    (void)dir;

    struct inode *inode = new_inode(sb);
    if (!inode)
        return NULL;

    inode->ino = ramfs_get_next_ino();
    inode->mode = mode;
    inode->type = mode & S_IFMT;
    atomic_set(&inode->ref, 1);

    switch (inode->type) {
        case S_IFDIR:
            inode->ops = &ramfs_iops;
            inode->links = 2;
            break;
        case S_IFREG:
            inode->links = 1;
            break;
        default:
            inode->links = 1;
            inode->private = (void *)(uintptr_t)dev;
            break;
    }

    return inode;
}



/* For ramfs, we will use the dentry tree for lookups */
static int ramfs_lookup(struct inode *dir, struct dentry *negative, u32 flags)
{
    return -1;
}

static int ramfs_create(struct inode *dir, struct dentry *negative, mode_t mode)
{
    struct inode *inode = ramfs_get_inode(dir->sb, dir, mode | S_IFREG, dir->sb->device);

    if (!inode)
        return -ENOMEM;

    d_instantiate(negative, inode);
    return 0;
}

static int ramfs_mkdir(struct inode *dir, struct dentry *negative, mode_t mode)
{
    struct inode *inode = ramfs_get_inode(dir->sb, dir, mode | S_IFDIR, dir->sb->device);

    if (!inode)
        return -ENOMEM;

    d_instantiate(negative, inode);
    return 0;
}

static int ramfs_rmdir(struct inode *dir, struct dentry *entry)
{
    if (!list_empty(&entry->subdirs))
        return -ENOTEMPTY;
    
    return 0;
}

static struct inode_ops ramfs_iops = {
    .lookup = ramfs_lookup,
    .create = ramfs_create,
    .mkdir  = ramfs_mkdir,
    .rmdir  = ramfs_rmdir,
};



static struct inode *ramfs_alloc_inode(struct superblock *sb)
{
    return (struct inode *)kmalloc(sizeof(struct inode), PAL_KERNEL);
}

static void ramfs_evict_inode(struct inode *inode)
{

}

static struct superblock_ops ramfs_sbops = {
    .alloc_inode = ramfs_alloc_inode,
    .evict_inode = ramfs_evict_inode
};



static int ramfs_fill_super(struct superblock *sb)
{
    sb->ops = &ramfs_sbops;
    struct qstr name = { .str = "/", .len = 1 };
    sb->root = d_alloc(NULL, &name);
    return 0;
}

static void ramfs_kill_super(struct superblock *sb)
{
}

static struct filesystem ramfs = {
    .name       = "ramfs",
    .flags      = 0,
    .fill_super = ramfs_fill_super,
    .kill_super = ramfs_kill_super,
};

extern struct mount mnt_root;
extern struct dentry d_root;

int init_ramfs()
{
    register_filesystem(&ramfs);
    if (!mount_bdev(&ramfs, 0, &mnt_root, &d_root))
        return -1;

    return 0;
}
