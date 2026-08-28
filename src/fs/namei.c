
#include <tiny/fs/namei.h>

extern struct mount  mnt_root;
extern struct dentry d_root;

static __force_inline u32 get_component_len(const char *name)
{
    u32 len = 0;
    while (name[len] && name[len] != '/') { len++; }
    return len;
}

int path_walk(const struct path *cwd, const char *path, struct path *out)
{
    if (path == NULL || out == NULL)
        return -EINVAL;

    struct path p = {0};

    if (path[0] == '/') {
        p.mnt = &mnt_root;
        p.dentry = &d_root;
        path++;
    }
    else {
        if (!cwd)
            return -EINVAL;

        p.dentry = cwd->dentry;
        p.mnt    = cwd->mnt;

        panic("not implemented yet");
    }

    for (;;) {
        u32 comp_len = get_component_len(path);

        if (comp_len == 1 && path[0] == '.') {
            goto advance;
        }
        else if (comp_len == 2 && path[0] == '.' && path[1] == '.') {
            if (p.dentry == p.mnt->root) {
                p.dentry = p.mnt->parent->root;
                p.mnt = p.mnt->parent;
            }
            else {
                p.dentry = p.dentry->parent;
            }
            goto advance;
        }

        struct qstr component = { .str = path, .len = comp_len };

        /* Mount pivoting */
        if (p.dentry->flags & DCACHE_MOUNTED) {
            p.mnt = vfs_lookup_mount(p.dentry);
            if (!p.mnt)
                return -ENOENT;
            
            p.dentry = p.mnt->root;
        }

        p.dentry = dlookup(p.dentry, &component);

        if (!p.dentry)
            return -ENODIR;

    advance:
        path += comp_len;
        while (*path == '/') { path++; }
        if (!*path) break;
    }

    *out = p;
    return 0;
}