
#ifndef NAMEI_H
#define NAMEI_H

#include <tiny/fs/vfs.h>
#include <tiny/string.h> 

struct nameidata {
    struct path path;
    struct qstr last;
};

int path_walk(const struct path *cwd, const char *path, struct path *out);

#endif // NAMEI_H