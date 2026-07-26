
#ifndef VFS_H
#define VFS_H

#include <arch/atomic.h>

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/errno.h>
#include <tiny/fs/inode.h>
#include <tiny/device/device.h>

#define MAX_PATH_SIZE 4096

struct superblock;
struct dentry;
struct path;
struct mount;

struct filesystem {
    const char *name;
    uint8_t flags;
    int (*init_fs)(struct superblock *);
    void (*kill_fs)(struct superblock *);
    struct list_head list;
};

struct superblock_ops {
    int (*statfs) (struct superblock *, void *buf);
    void (*put_super) (struct superblock *);
};

struct superblock {
    struct list_head list;
    dev_t device;

    struct filesystem *fs;
    struct superblock_ops *ops;
    struct dentry *root;

    struct list_head inodes;
};

struct dentry {
    struct inode *inode;
    struct qstr name;

    struct dentry *parent;
    struct list_head child;
    struct list_head subdirs;

    atomic_t ref;
};

struct mount {
    struct mount *parent;
    struct superblock *sb;
    struct dentry *mountpoint;
    struct list_head child;
    struct list_head sub_mnts;
};

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);

#endif // VFS_H
