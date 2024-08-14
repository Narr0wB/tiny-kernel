
#ifndef STRING_H
#define STRING_H

#include <common.h>

void* memcpy(void *dest, const void *src, size_t count);
void* memset(void *dest, int c, size_t count);
void* memmove(void *dest, const void *src, size_t count);
int memcmp(const void *buf1, const void *buf2, size_t count);

int is_digit(char c);
int digit(char c);

#endif // STRING_H
