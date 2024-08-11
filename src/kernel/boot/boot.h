
#ifndef BOOT_H
#define BOOT_H 

#define FB_OFFSET 0

#include <common.h>

typedef struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
} framebuffer_t;

typedef struct bootinfo {
    framebuffer_t framebuffer;
} bootinfo_t;

#endif // BOOT_H

