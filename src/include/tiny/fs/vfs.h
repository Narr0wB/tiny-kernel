
#ifndef VFS_H
#define VFS_H

#include <arch/atomic.h>

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/hashtable.h>
#include <tiny/errno.h>
#include <tiny/fs/inode.h>
#include <tiny/device/device.h>

DEFINE_HASHTABLE(mnthash, 5);

#define DCACHE_MOUNTED (1U << 0)

struct superblock;
struct dentry;
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

    uint32_t flags;

    atomic_t ref;
};

struct mount {
    struct mount *parent;
    struct superblock *sb;
    struct dentry *mountpoint;
    struct list_head child;
    struct list_head sub_mnts;
};

static __force_inline int d_is_dir(struct dentry *ent) 
{
    return ent->inode->flags == INO_DIRECTORY;
}

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);



int graft_tree(struct mount *mnt, struct mount *parent, struct dentry *mountpoint);
int cut_tree(struct dentry *mountpoint);

#endif // VFS_H
