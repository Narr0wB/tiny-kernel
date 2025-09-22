
#ifndef VFS_H
#define VFS_H

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/errno.h>
#include <arch/atomic.h>

#define MAX_PATH_SIZE 4096

struct superblock;
struct inode;
struct dentry;
struct path;
struct mount;

struct superblock_ops {
    int (*statfs) (struct superblock *, void *buf);
    void (*put_super) (struct superblock *);
};

struct inode_ops {
    int (*create) (struct inode *, const char *, int, int, struct inode **);
};

struct superblock {
    struct superblock_ops *ops;
    struct dentry *root;
    void *fs_private;
};

struct inode {
    atomic_t ref;
    struct inode_ops *ops;
    uint64_t ino;
    uint32_t mode;
    size_t size;
};

struct dentry {
    atomic_t ref;
    struct dentry *parent;
    struct inode *inode;
    char *name;
};

struct path {
    struct mount *mnt;
    struct dentry *entry;
};

struct mount {
    struct superblock *sb;
    struct dentry *mp_dentry;
    struct dentry *root; // It's just sb->root
    struct mount *parent;
};

typedef struct {
    struct inode *node;
    uint64_t file_pos;
    int flags;
} FILE;

// int register_filesystem(struct fs *fs);
// int unregister_filesystem(struct fs *fs);

// int mount_filesystem(struct mount *mnt);
// int umount_filesystem(const char *name); 

FILE *open(const char *name, int flags);
int read(FILE *file, const void *dest, size_t len);
int write(FILE *file, const void *src, size_t len);
void close(FILE *file);

#endif // VFS_H
