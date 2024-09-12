
#include <fs/vfs.h>

static struct inode vfs_root;
static struct mount *mounted_fs[MAX_MOUNT];
static size_t current_mounted;

int mount_filesystem(struct mount *mnt) {
    kassert(mnt != NULL);
        
    // Check that the filesystem that we are mounting is not mounted on the same root as another mounted filesystem
    for (size_t i = 0; i < current_mounted; ++i) {
        if (mounted_fs[i] == mnt) {
            return -EEXIST;
        }
        if (mounted_fs[i]->root == mnt->root) {
            return -EINVAL;
        }
    }
    
    mnt->fs->root = mnt->root; 

    mounted_fs[current_mounted] = mnt;
    return 0;
}

int umount_filesystem(struct mount *mnt) {
    kassert(mnt != NULL);
    bool found = false;

    for (size_t i = 0; i < current_mounted; ++i) {
        if (mounted_fs[i] == mnt) {
            // Temporarily switch fount mnt with the last mount in the array
            struct mount *tmp = mounted_fs[i];
            mounted_fs[i] = mounted_fs[current_mounted - 1];
            mounted_fs[current_mounted - 1] = tmp;

            // Now evict the last element of the array
            current_mounted--;
        } 
    }

    if (!found) {
        return -ENOENT;
    }

    return 0;
}
