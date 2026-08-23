
#include <tiny/fs/namei.h>

extern struct mount  mnt_root;
extern struct dentry d_root;

static __force_inline u32 get_component_len(const char *name)
{
    u32 len = 0;
    do { len++; } while (name[len] != '/' && name[len]);
    return len;
}

int path_walk(struct path cwd, const char *path, struct dentry **out)
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
        p = cwd;
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
        p.dentry = d_lookup(p.dentry, &component);
        if (!p.dentry)
            return -ENODIR;

    advance:
        path += comp_len;

        if (!*path) break;
        while (*path == '/') { path++; }
    }

    *out = p.dentry;
    return 0;
}