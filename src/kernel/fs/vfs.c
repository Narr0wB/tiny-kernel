
#include <fs/vfs.h>

static struct mount *roots[MAX_MOUNTS];
static size_t current_mounted;

int mount_filesystem(struct mount *mnt) {
    kassert(mnt != NULL);
        
    // Check that the filesystem that we are mounting is not mounted on the same root as another mounted filesystem
    for (size_t i = 0; i < current_mounted; ++i) {
        if (strcmp(roots[i]->fs->name, mnt->fs->name)) {
            return -EEXIST;
        }
    }
    
    roots[++current_mounted] = mnt;
    return 0;
}

int umount_filesystem(const char *name) {
    kassert(name != NULL);

    bool found = false;

    for (size_t i = 0; i < current_mounted; ++i) {
        if (strcmp(roots[i]->fs->name, name) == 0) {
            // Temporarily switch fount mnt with the last mount in the array
            struct mount *tmp = roots[i];
            roots[i] = roots[current_mounted - 1];
            roots[current_mounted - 1] = tmp;

            // Now evict the last element of the array
            current_mounted--;
        } 
    }

    if (!found) {
        return -ENOENT;
    }

    return 0;
}
