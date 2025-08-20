
#ifndef VFS_H
#define VFS_H

#include <common.h>
#include <util/assert.h>
#include <sys/errno.h>
#include <fs/inode.h>

#define MAX_PATH_SIZE 4096
#define MAX_MOUNTS    256

struct inode;
struct dentry;
struct mount;
struct fs;
struct superblock;

struct mount {
    struct inode *root;
    struct fs    *fs;
};

struct fs {
    const char *name;
    int (*mount) (struct mount *, struct fs *);
};

typedef struct {
    struct inode *node;
    uint64_t file_pos;
    int flags;
} FILE;

int register_filesystem(struct fs *fs);
int unregister_filesystem(struct fs *fs);

int mount_filesystem(struct mount *mnt);
int umount_filesystem(const char *name); 

FILE *open(const char *name, int flags);
int read(FILE *file, const void *dest, size_t len);
int write(FILE *file, const void *src, size_t len);
void close(FILE *file);

#endif // VFS_H
