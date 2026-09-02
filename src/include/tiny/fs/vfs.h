
#ifndef VFS_H
#define VFS_H

#include <arch/atomic.h>

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/hashtable.h>
#include <tiny/errno.h>
#include <tiny/string.h>
#include <tiny/fs/inode.h>
#include <tiny/device/device.h>

/* Superblock flags */
#define SB_ACTIVE (1U << 0)

/* Dentry flags */
#define DCACHE_MOUNTED (1U << 0)

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

struct superblock;
struct dentry;
struct mount;
struct inode;
struct inode_ops;
struct file;
struct stat;
struct statfs;

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

void register_filesystem(struct filesystem *fs);
void unregister_filesystem(const char *name);


struct superblock_ops {
    struct inode *(*alloc_inode)(struct superblock *);
    void          (*free_inode)(struct inode *);
    int           (*write_inode)(struct inode *);
    void          (*evict_inode)(struct inode *);
    int           (*statfs) (struct superblock *, struct statfs *);
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
    void                  *private;
};




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

struct dentry *dalloc(struct dentry *parent, struct qstr *name);
struct dentry *dlookup(struct dentry *parent, struct qstr *name);
void           ddelete(struct dentry *dir);

static __force_inline void dinstantiate(struct dentry *dentry, struct inode *inode)
{
    // atomic_inc(&inode->ref);
    dentry->inode = inode;
}

static __force_inline struct dentry *dget(struct dentry *dentry)
{
    atomic_inc(&dentry->ref);
    return dentry;
}

static __force_inline void dput(struct dentry *dentry)
{
    if (!atomic_dec_and_test(&dentry->ref))
        return;

    ddelete(dentry);
}



struct inode_ops {
    int (*lookup)(struct inode *, struct dentry *, u32);
    int (*create)(struct inode *, struct dentry *, mode_t);
    int (*mkdir)(struct inode *, struct dentry *, mode_t);
    int (*rmdir)(struct inode *, struct dentry *);
    int (*rename)(struct inode *, struct dentry *, struct inode *, struct dentry *, u32);
    int (*truncate)(struct inode *);
};

struct file_ops {
    int     (*open)(struct inode *, struct file **, u32);
    void    (*release)(struct file *);
    ssize_t (*read)(struct file *, void *, size_t);
    ssize_t (*write)(struct file *, const void *, size_t);
};

struct inode {
    ino_t              ino;
    struct superblock *sb;
    mode_t             mode;
    size_t             size;
    u32                links;
    time_t             atime, mtime, ctime;
    struct inode_ops  *ops;
    struct file_ops   *fops;
    struct list_head   sb_list;
    struct hlist_node  hnode;
    void              *private;
    atomic_t           ref;
};

struct file {
    struct inode    *inode;
    struct file_ops *ops;
    u32              flags;
    u32              mode;
    loff_t           pos;
};

struct inode *iget(struct superblock *sb, ino_t ino);
void          iput(struct inode *ino);



struct mount {
    struct mount      *parent;
    struct superblock *sb;
    struct dentry     *mountpoint;
    struct dentry     *root;
    struct list_head   child;
    struct list_head   submnts;
    struct hlist_node  hnode;
};

struct path {
    struct mount  *mnt;
    struct dentry *dentry;
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


struct mount *vfs_mount(struct filesystem *fs, dev_t dev, struct path *path);
int vfs_umount(struct dentry *mountpoint);
struct mount *vfs_lookup_mount(struct dentry *mountpoint);

int vfs_open(const struct path *path, struct file **result, u32 flags);
int vfs_close(struct file *f);

ssize_t vfs_read(struct file *f, void *buf, size_t sz);
ssize_t vfs_write(struct file *f, const void *buf, size_t sz);

int vfs_getattr(const struct path *path, struct stat *stat);
int vfs_statfs(struct superblock *sb, struct statfs *stat);

int vfs_create(struct inode *parent, struct dentry *negative, mode_t mode);
int vfs_remove(struct inode *parent, struct dentry *dentry);
int vfs_mkdir(struct inode *parent, struct dentry *negative, mode_t mode);
int vfs_rmdir(struct inode *parent, struct dentry *dir);

void init_vfs();

#endif // VFS_H