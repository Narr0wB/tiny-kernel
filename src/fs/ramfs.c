
#include <tiny/mm/kmalloc.h>

#include <tiny/fs/ramfs.h>

static u32 ramfs_monotonic_count = 0;

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
    return 0;
}

static int ramfs_create(struct inode *dir, struct dentry *negative, mode_t mode)
{
    struct inode *inode = ramfs_get_inode(dir->sb, dir, mode, dir->sb->device);
}

static int ramfs_mkdir(struct inode *dir)
{

}

static int ramfs_rmdir(struct inode *dir)
{

}

static const struct inode_ops ramfs_iops = {
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

static const struct superblock_ops ramfs_sbops = {
    .alloc_inode = ramfs_alloc_inode,
    .evict_inode = ramfs_evict_inode
};



static int ramfs_fill_super(struct superblock *sb)
{
    return 0;
}

static void ramfs_kill_super(struct superblock *sb)
{

}

static const struct filesystem ramfs = {
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
    int err = mount_bdev(&ramfs, 0, &mnt_root, &d_root);
    return err;
}
