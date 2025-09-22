
#include <tiny/fs/vfs.h>
#include <tiny/mm/palloc.h>

// We are going to use a single mount namespace
static struct mount root_mount;

static struct inode_ops ramfs_inode_ops = {0};

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

// int mount_filesystem(struct mount *mnt) {
//     kassert(mnt != NULL);
//
//     // Check that the filesystem that we are mounting is not mounted on the same root as another mounted filesystem
//     for (size_t i = 0; i < current_mounted; ++i) {
//         if (strcmp(roots[i]->fs->name, mnt->fs->name)) {
//             return -EEXIST;
//         }
//     }
//
//     roots[++current_mounted] = mnt;
//     return 0;
// }
//
// int umount_filesystem(const char *name) {
//     kassert(name != NULL);
//
//     bool found = false;
//
//     for (size_t i = 0; i < current_mounted; ++i) {
//         if (strcmp(roots[i]->fs->name, name) == 0) {
//             // Temporarily switch fount mnt with the last mount in the array
//             struct mount *tmp = roots[i];
//             roots[i] = roots[current_mounted - 1];
//             roots[current_mounted - 1] = tmp;
//
//             // Now evict the last element of the array
//             current_mounted--;
//         } 
//     }
//
//     if (!found) {
//         return -ENOENT;
//     }
//
//     return 0;
// }