
#ifndef VFS_H
#define VFS_H

#include <arch/atomic.h>

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/hashtable.h>
#include <tiny/errno.h>
#include <tiny/fs/inode.h>
#include <tiny/device/device.h>

struct superblock;
struct dentry;
struct mount;

struct filesystem {
    const char *name;
    uint8_t flags;
    struct list_head list;

    int (*fill_super)(struct superblock *);
    void (*kill_super)(struct superblock *);
};

struct superblock_ops {
    int (*statfs) (struct superblock *, void *);
    void (*put_super) (struct superblock *);
    int (*alloc_inode) (struct superblock *);
    int (*destroy_inode) (struct superblock *, struct inode *);
};

struct superblock {
    struct superblock_ops *ops;

    struct list_head list;
    struct list_head inodes;

    struct filesystem *fs;
    dev_t device;
    uint32_t flags;

    struct dentry *root;
    atomic_t count;
};

/* Superblock flags */
#define SB_ACTIVE (1U << 0)

struct dentry {
    struct inode *inode;
    struct qstr name;

    struct dentry *parent;
    struct list_head child;
    struct list_head subdirs;

    uint32_t flags;

    atomic_t ref;
};

/* Dentry flags */
#define DCACHE_MOUNTED (1U << 0)

struct mount {
    struct mount *parent;
    struct superblock *sb;
    struct dentry *mountpoint;
    struct dentry *root;
    struct list_head child;
    struct list_head sub_mnts;
    struct hlist_node hnode;
};

static __force_inline int d_is_dir(struct dentry *ent) 
{
    return ent->inode->flags == INO_DIRECTORY;
}

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);


struct mount *mount_bdev(struct filesystem *fs, dev_t dev, struct mount *parent, struct dentry *mountpoint);
void umount_bdev(struct dentry *mountpoint);

#endif // VFS_H
