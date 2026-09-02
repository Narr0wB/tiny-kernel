
#ifndef COMPILER_H
#define COMPILER_H

#include <tiny/types.h>

#define __force_inline inline __attribute__((always_inline))
#define __packed __attribute__((packed))
#define __weak __attribute__((weak))
#define __align(x) __attribute__((aligned(x)))

#define barrier() __asm__ volatile ("" ::: "memory")

#define ilog2(n) \
    ( sizeof(n) <= 4 ? 31 - __builtin_clz(n) : 63 - __builtin_clzll(n) )

static __force_inline void *memcpy(void *dest, const void *src, size_t count) {
    if (!count || dest == src)
        return dest;
    
    for (size_t i = 0; i < count; ++i) {
        *((unsigned char*) dest + i) = *((unsigned char*) src + i);
    }

    return dest;
}

static __force_inline void *memset(void *dest, int c, size_t count) {
    if (!count) return dest;

    for (size_t i = 0; i < count; ++i) {
        *((unsigned char*) dest + i) = (unsigned char)c;
    }

    return dest;
}

static __force_inline void *memmove(void *dest, const void *src, size_t count) {
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

static __force_inline int memcmp(const void *buf1, const void *buf2, size_t count) {
    if (!count) return 0;

    while (--count && *(unsigned char*)buf1 == *(unsigned char*)buf2) {
        buf1 = ((unsigned char*)buf1 + 1);
        buf2 = ((unsigned char*)buf2 + 1);
    }

    return *(unsigned char*)buf1 - *(unsigned char*)buf2;
}


/*
 * Write into p the value stored in v that is size-bytes in a way so that the compiler wont interfere
 */
static __force_inline void __write_once_size(volatile void *p, const void *v, int size)
{
    switch (size) {
        case 1: *(volatile uint8_t *)p  = *(const uint8_t *)v;  break;
        case 2: *(volatile uint16_t *)p = *(const uint16_t *)v; break;
        case 4: *(volatile uint32_t *)p = *(const uint32_t *)v; break;
        case 8: *(volatile uint64_t *)p = *(const uint64_t *)v; break;
        default:
            barrier();
            memcpy((void *)p, v, size);
            barrier();
    }
}

#define WRITE_ONCE(x, v) ({\
    typeof(x) __v = (typeof(x))v; \
    __write_once_size(&(x), &__v, sizeof(typeof(x))); \
    __v; \
})

static __force_inline u16 read_le16(void *addr)
{
    return *((u16*)addr);
}

static __force_inline u32 read_le32(void *addr)
{
    return *((u32*)addr);
}

#endif // COMPILER_H