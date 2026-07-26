
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>

#include <tiny/fs/vfs.h>

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);
static struct mount *mounts; 

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

int init_vfs() {
    // Create a ramfs instance
    // struct superblock *root_sb = (struct superblock *)kpalloc();
    // struct superblock_ops *root_sb_ops = (struct superblock_ops *)kpalloc();
    // struct dentry *root_dentry = (struct dentry *)kpalloc();

    // root_sb->ops = root_sb_ops;
    // root_sb->root = root_dentry;
    // root_sb->fs_private = NULL;
    
    // root_mount.sb = root_sb;
    // root_mount.mp_dentry = NULL; // Since this is the root mount of the entire OS, it's mountpoint entry is NULL
    // root_mount.root = root_dentry;
    // root_mount.parent = NULL;
}