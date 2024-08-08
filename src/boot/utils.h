
#ifndef UTILS_H
#define UTILS_H

#include <efi.h>

typedef struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
} framebuffer_t;

#endif // UTILS_H
