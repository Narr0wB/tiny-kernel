
#include <klibc/string.h>

void* memcpy(void *dest, const void *src, size_t count) {
    if (!count || dest == src)
        return dest;
    
    for (size_t i = 0; i < count; ++i) {
        *((unsigned char*) dest + i) = *((unsigned char*) src + i);
    }

    return dest;
}

void* memset(void *dest, int c, size_t count) {
    if (!count) return dest;

    for (size_t i = 0; i < count; ++i) {
        *((unsigned char*) dest + i) = (unsigned char)c;
    }

    return dest;
}

void* memmove(void *dest, const void *src, size_t count) {
    if (!count || dest == src) 
        return dest;
   
    if (dest < src) {
        for (size_t i = 0; i < count; ++i) {
            ((unsigned char*)dest)[i] = ((unsigned char*)src)[i];
        }
    }
    else {
        for (size_t i = 1; i < count; ++i) {
            ((unsigned char*)dest)[count - i] = ((unsigned char*)src)[count - i];
        }
    }

    return dest;
}

int memcmp(const void *buf1, const void *buf2, size_t count) {
    if (!count) return 0;

    while (--count && *(unsigned char*)buf1 == *(unsigned char*)buf2) {
        buf1 = ((unsigned char*)buf1 + 1);
        buf2 = ((unsigned char*)buf2 + 1);
    }

    return *(unsigned char*)buf1 - *(unsigned char*)buf2;
}

int is_digit(char c) {
    if ('0' <= c && c <= '9') return 1;
    return 0;
}

int digit(char c) {
    if (!is_digit(c)) return 0;
    return (c - '0');
}
