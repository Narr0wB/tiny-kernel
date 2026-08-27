
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

typedef u32 ino_t;
typedef u32 mode_t;
typedef u64 time_t;
typedef u64 loff_t;

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

    int           (*statfs) (struct superblock *, void *);
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
    struct inode     *inode;
    struct qstr       name;

    struct dentry    *parent;
    struct hlist_node hnode;
    struct list_head  child;
    struct list_head  subdirs;

    u32               flags;
    atomic_t          ref;
};

/* Dentry flags */
#define DCACHE_MOUNTED (1U << 0)

struct dentry *d_alloc(struct dentry *parent, struct qstr *name);
void           d_instantiate(struct dentry *dentry, struct inode *inode);
struct dentry *d_lookup(struct dentry *parent, struct qstr *name);
void           d_delete(struct dentry *dir);
struct dentry *dget(struct dentry *dentry);
void           dput(struct dentry *dentry);

struct inode_ops {
    int (*lookup)(struct inode *, struct dentry *, u32);
    int (*create)(struct inode *, struct dentry *, mode_t);
    int (*mkdir)(struct inode *, struct dentry *, mode_t);
    int (*rmdir)(struct inode *, struct dentry *);
    int (*unlink)(struct inode *, struct dentry *);
    int (*rename)(struct inode *, struct dentry *, struct inode *, struct dentry *, u32);
    int (*truncate)(struct inode *);
};

struct file_ops {

};

struct inode {
    ino_t              ino;
    struct superblock *sb;
    mode_t             mode;
    size_t             size;
    u32                links;

    time_t             atime, mtime, ctime;
    u32                type;

    atomic_t           ref;

    struct inode_ops  *ops;
    struct file_ops   *fops;

    struct list_head   sb_list;
    struct hlist_node  hnode;

    void              *private;
};

struct file {
    struct dentry   *dentry;
    struct mount    *mount;
    struct file_ops *ops;
    u32              flags;
    u32              mode;
    loff_t           pos;
};

#define S_IFMT   0170000   /* type mask */
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000   /* regular file */
#define S_IFBLK  0060000   /* block device */
#define S_IFDIR  0040000   /* directory */
#define S_IFCHR  0020000   /* char device */
#define S_IFIFO  0010000   /* fifo */

#define S_ISUID  04000     /* set-uid */
#define S_ISGID  02000     /* set-gid */
#define S_ISVTX  01000     /* sticky  */

#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)

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
    return S_ISDIR(ent->inode->mode);
}

static __force_inline struct inode *alloc_inode(struct superblock *sb)
{
    return sb->ops->alloc_inode(sb);
}

static __force_inline struct inode *new_inode(struct superblock *sb)
{
    struct inode *inode = alloc_inode(sb);
    if (!inode)
        return NULL;

    memset(inode, 0, sizeof(struct inode));
    inode->sb = sb;
    list_add(&inode->sb_list, &sb->inodes);

    return inode;
}

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);

struct inode *iget(struct superblock *sb, ino_t ino);
int iput(struct inode *inode);

struct mount *mount_bdev(struct filesystem *fs, dev_t dev, struct mount *parent, struct dentry *mountpoint);
int umount_bdev(struct dentry *mountpoint);

struct statfs {

};

int vfs_open(struct dentry *file);
int vfs_close(struct dentry *file);
int vfs_read(struct dentry *file, size_t pos, void *buf, size_t sz);
int vfs_write(struct dentry *file, size_t pos, void *buf, size_t sz);
int vfs_stat(struct dentry *file, struct statfs *stats);
int vfs_mkdir(struct dentry *parent);
int vfs_rmdir(struct dentry *dir);

#endif // VFS_H
