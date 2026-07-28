
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/list.h>
#include <tiny/errno.h>

#include <tiny/fs/vfs.h>

static LIST_HEAD(filesystems);
static LIST_HEAD(superblocks);
static LIST_HEAD(d_root);
static struct mount *mounts;

void register_filesystem(struct filesystem *fs)
{
    list_add(fs, &filesystems);
}

void unregister_filesystem(const char *name)
{
    struct filesystem *fs;
    list_foreach_entry(&filesystems, fs, list) {
        if (strcmp(fs->name, name) == 0)
            list_del(&fs->list);
    }
}

int graft_tree(struct mount *mnt, struct mount *parent, struct dentry *mountpoint)
{
    if (!d_is_dir(mountpoint))
        return -ENODIR;

    mountpoint->flags |= DCACHE_MOUNTED;

    mnt->mountpoint = mountpoint;
    mnt->parent = parent;

    list_add_tail(&mnt->child, &parent->sub_mnts);

    return 0;
}

int cut_tree(struct dentry *mountpoint)
{
    if (!(mountpoint->flags & DCACHE_MOUNTED))
        return -EINVAL;
    
    
}

int init_vfs() 
{
}