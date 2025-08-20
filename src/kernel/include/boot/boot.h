
#ifndef BOOT_H
#define BOOT_H 

#include <common.h>
#include <memory/types.h>

typedef struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
} framebuffer_t;

typedef struct bootinfo {
    framebuffer_t framebuffer;
    memory_map_t map;
    paddr_t kernel_start;
    paddr_t kernel_end;
} bootinfo_t; 

#endif // BOOT_H

