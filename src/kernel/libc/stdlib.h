
#ifndef STDLIB_H
#define STDLIB_H

#include <common.h>

#define get_bit(v, n) ((v >> n) & 1)

void* memcpy(void *dest, const void *src, size_t count);
void* memset(void *dest, int c, size_t count);
void* memmove(void *dest, const void *src, size_t count);
int memcmp(const void *buf1, const void *buf2, size_t count);

#endif // STDLIB_H
