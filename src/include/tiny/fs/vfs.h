
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
struct inode;
struct inode_ops;
struct file;

typedef enum {
    INO_VALID,
    INO_DIRECTORY,
    INO_BAD,
} iflags_t;

typedef uint32_t ino_t;
typedef uint32_t mode_t;
typedef uint64_t time_t;

struct filesystem {
    const char      *name;
    uint8_t          flags;
    struct list_head list;

    int  (*fill_super)(struct superblock *);
    void (*kill_super)(struct superblock *);
};

struct superblock_ops {
    struct inode *(*alloc_inode)(struct superblock *);
    void          (*destroy_inode)(struct inode *);
    void          (*free_inode)(struct inode *);

    void          (*dirty_inode)(struct inode *, int);
    int           (*write_inode)(struct inode *);
    int           (*drop_inode)(struct inode *);
    void          (*evict_inode)(struct inode *);

    int  (*statfs) (struct superblock *, void *);
};

struct superblock {
    struct superblock_ops *ops;
    struct list_head       list;
    struct list_head       inodes;
    struct filesystem     *fs;
    dev_t                  device;
    uint32_t               flags;
    struct dentry         *root;
    atomic_t               count;
};

/* Superblock flags */
#define SB_ACTIVE (1U << 0)


struct dentry {
    struct inode    *inode;
    struct qstr      name;

    struct dentry   *parent;
    struct list_head child;
    struct list_head subdirs;

    uint32_t         flags;
    atomic_t         ref;
};

/* Dentry flags */
#define DCACHE_MOUNTED (1U << 0)

struct inode_ops {
    struct dentry *(*lookup)(struct inode *dir, struct dentry *dentry, int flags);
    // int (*lookup) (struct inode *dir, const char *name, struct inode **result);
    // int (*create) (struct inode *dir, const char *name, mode_t mode, struct inode **result);
    // int (*mkdir) (struct inode *, const char *name, mode_t mode, struct inode **result);
    // int (*rmdir) (struct inode *dir, struct inode *entity, const char *name);
    // int (*truncate) (struct inode *);
    // int (*link) (struct inode *dir, const char *name, struct inode **result);
    // int (*unlink) (struct inode *dir, const char *name, struct inode **result);
};

struct inode {
    ino_t              ino;
    struct superblock *sb;
    mode_t             mode;
    size_t             size;
    uint32_t           links;

    time_t             atime, mtime, ctime;
    uint32_t           type;

    atomic_t           ref;

    struct inode_ops  *ops;
    struct file_ops   *fops;

    struct list_head   sb_list;
    struct hlist_node  hnode;

    void              *private;
};

struct mount {
    struct mount      *parent;
    struct superblock *sb;
    struct dentry     *mountpoint;
    struct dentry     *root;
    struct list_head   child;
    struct list_head   sub_mnts;
    struct hlist_node  hnode;
};

static __force_inline int d_is_dir(struct dentry *ent) 
{
    return ent->inode->mode == INO_DIRECTORY;
}

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);

struct inode *iget(struct superblock *sb, ino_t ino);
int iput(struct inode *inode);

struct mount *mount_bdev(struct filesystem *fs, dev_t dev, struct mount *parent, struct dentry *mountpoint);
int umount_bdev(struct dentry *mountpoint);

#endif // VFS_H
