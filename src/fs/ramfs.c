
#include <tiny/fs/vfs.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/compiler.h>
#include <tiny/string.h>
#include <tiny/list.h>

#include <tiny/fs/ramfs.h>


static u32 ramfs_monotonic_count = 0;
static struct inode_ops ramfs_iops;
static struct file_ops  ramfs_fops;

static u32 ramfs_get_next_ino()
{
    return ramfs_monotonic_count++;
}

static struct inode *ramfs_get_inode(struct superblock *sb, struct inode *dir, mode_t mode, bdev_t dev)
{
    (void)dir;

    struct inode *inode = new_inode(sb);
    if (!inode)
        return NULL;

    inode->ino  = ramfs_get_next_ino();
    inode->mode = mode;
    atomic_set(&inode->ref, 1);

    switch (mode & S_IFMT) {
        case S_IFDIR:
            inode->ops = &ramfs_iops;
            inode->links = 2;
            break;
        case S_IFREG:
            inode->fops = &ramfs_fops;
            inode->links = 1;
            break;
        default:
            inode->fops = &ramfs_fops;
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

    dinstantiate(negative, inode);
    return 0;
}

static int ramfs_mkdir(struct inode *dir, struct dentry *negative, mode_t mode)
{
    struct inode *inode = ramfs_get_inode(dir->sb, dir, mode | S_IFDIR, dir->sb->device);

    if (!inode)
        return -ENOMEM;

    dinstantiate(negative, inode);
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



static int ramfs_file_open(struct inode *ino, struct file **out, u32 flags)
{
    struct file *file = (struct file *)kmalloc(sizeof(struct file), PAL_KERNEL);
    if (!file)
        return -ENOMEM;

    file->inode  = ino;
    file->ops    = &ramfs_fops;
    file->flags  = flags;
    file->mode   = ino->mode;
    file->pos    = 0;

    return 0;
}

static void ramfs_file_release(struct file *file)
{
    kfree(file);
}

static ssize_t ramfs_file_read(struct file *file, void *buf, size_t sz)
{
    return 0;
}

static ssize_t ramfs_file_write(struct file *file, const void *buf, size_t sz)
{
    return 0;
}

static struct file_ops ramfs_fops = {
    .open    = ramfs_file_open,
    .release = ramfs_file_release,
    .read    = ramfs_file_read,
    .write   = ramfs_file_write,
};



static __force_inline struct inode *ramfs_alloc_inode(struct superblock *sb)
{
    return (struct inode *)kmalloc(sizeof(struct inode), PAL_KERNEL);
}

static __force_inline void ramfs_free_inode(struct inode *inode)
{
    if (!inode)
        return;
    kfree(inode);
}

static __force_inline int ramfs_statfs(struct superblock *sb, struct statfs *stat)
{
    return 0;
}

static struct superblock_ops ramfs_sbops = {
    .alloc_inode   = ramfs_alloc_inode,
    .free_inode    = ramfs_free_inode,
    .write_inode   = NULL,
    .evict_inode   = NULL,
    .statfs        = ramfs_statfs
};



static int ramfs_fill_super(struct superblock *sb)
{
    sb->ops = &ramfs_sbops;
    sb->root = dalloc(NULL, NULL);
    if (!sb->root)
        return -ENOMEM;

    struct inode *root_ino = ramfs_get_inode(sb, NULL, S_IFDIR, sb->device);
    if (!root_ino) {
        kfree(sb->root);
        return -ENOMEM;
    }

    dinstantiate(sb->root, root_ino);
    return 0;
}

static void ramfs_kill_super(struct superblock *sb)
{
    iput(sb->root->inode);
    dput(sb->root);
}

static struct filesystem ramfs = {
    .name       = "ramfs",
    .flags      = 0,
    .fill_super = ramfs_fill_super,
    .kill_super = ramfs_kill_super,
};



extern struct dentry droot;

void init_ramfs()
{
    register_filesystem(&ramfs);

    struct path p = { .mnt = NULL, .dentry = &droot };
    struct mount *mnt = vfs_mount(&ramfs, 0, &p);

    struct qstr name = QSTR("tmp");
    struct dentry *negative = dalloc(mnt->root, &name);
    vfs_mkdir(mnt->root->inode, negative, 0);

    name = QSTR("disk");
    negative = dalloc(mnt->root, &name);
    vfs_mkdir(mnt->root->inode, negative, 0);
}
