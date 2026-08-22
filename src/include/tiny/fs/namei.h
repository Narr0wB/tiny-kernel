
#ifndef NAMEI_H
#define NAMEI_H

#include <tiny/fs/vfs.h>
#include <tiny/string.h> 

struct path {
    struct mount  *mnt;
    struct dentry *dentry;
};

struct nameidata {
    struct path path;
    struct qstr last;
};

int path_walk(struct path cwd, const char *path, struct dentry **out);

#endif // NAMEI_H