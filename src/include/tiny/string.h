
#ifndef STRING_H
#define STRING_H

#include <tiny/types.h>
#include <tiny/assert.h>
#include <tiny/compiler.h>

int is_digit(char c);
int digit(char c);

int strcmp(const char *str1, const char *str2);
static __force_inline int strlen(const char *string) 
{
    int count = 0;
    while (string[count++] != 0) {}
    return count - 1;
}

struct qstr {
    char *str;
    size_t len;
};

#define QSTR(string) \
    (struct qstr){ .str = (string), .len = strlen(string) }

static __force_inline bool qstr_cmp(const struct qstr *a, const struct qstr *b)
{
    if (a->len != b->len)
        return false;
    return memcmp(a->str, b->str, a->len) == 0;
}

#endif // STRING_H
