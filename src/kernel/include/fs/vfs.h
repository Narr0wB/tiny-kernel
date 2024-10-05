
#ifndef VFS_H
#define VFS_H

#include <common.h>
#include <errno.h>
#include <util/assert.h>

#define MAX_PATH_SIZE 4096
#define MAX_NAME_SIZE 256
#define MAX_MOUNT     256

typedef uint32_t umode_t;

struct inode;
struct dentry;
struct mount;
struct fs;
struct superblock;
struct inode_ops;

struct inode_ops {
    int (*lookup) (struct inode *dir, struct inode **target, const char *path);
    int (*create) (struct inode *dir, struct inode **target, const char *path);
};

struct dentry {
    const char *path;
    struct inode *inode;
    struct dentry *children;
};

struct inode {
    uint32_t type;
    umode_t mode;
    struct mount *parent;
    struct inode_ops *v_ops;
    void *data;
};

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
int umount_filesystem(struct mount *mnt); 

FILE *open(const char *name, int flags);
int read(FILE *file, const void *dest, size_t len);
int write(FILE *file, const void *src, size_t len);
void close(FILE *file);

#endif // VFS_H
