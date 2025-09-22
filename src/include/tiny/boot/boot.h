
#ifndef BOOT_H
#define BOOT_H 

#include <tiny/types.h>
#include <tiny/mm/types.h>

struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
};

struct bootinfo {
    struct framebuffer framebuffer;
    struct efi_memory_map map;
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;
    paddr_t boot_vars_start;
    paddr_t boot_vars_end;
};   

#endif // BOOT_H

