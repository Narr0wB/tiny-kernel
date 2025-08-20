
#ifndef INODE_H
#define INODE_H

#include <common.h>

#define MAX_NAME_SIZE 256

struct inode;
struct inode_ops;
struct file_ops;
struct dentry;

typedef enum {
    INO_VALID,
    INO_BAD
} iflags_t;

typedef uint32_t ino_t;
typedef uint32_t mode_t;

struct inode {
    // Unique inode number
    ino_t ino;

    size_t size;
    mode_t mode;
    uint32_t links;
    uint32_t blocks;
    uint32_t block_size;

    time_t atime, mtime, ctime;

    iflags_t flags;

    uint32_t type;

    struct inode_ops *ops;
    struct file_ops *fops;
};

struct inode_ops {
    int (*lookup) (struct inode *dir, const char *name, struct inode **result);
    int (*create) (struct inode *dir, const char *name, mode_t mode, struct inode **result);
    int (*mkdir) (struct inode *, const char *name, mode_t mode, struct inode **result);
    int (*rmdir) (struct inode *dir, struct inode *entity, const char *name);

    // int (*link) (struct inode *dir, const char *name, struct inode **result);
    // int (*unlink) (struct inode *dir, const char *name, struct inode **result);
};

struct file_ops {

};

struct dentry {
    const char name[MAX_NAME_SIZE];
    ino_t ino;
} __attribute__((packed));

#endif // INODE_H