
#ifndef BOOT_H
#define BOOT_H 

#include <types.h>
#include <memory/types.h>

struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
};

struct bootinfo {
    struct framebuffer framebuffer;
    memory_map_t map;
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;
    paddr_t boot_variables_start;
    paddr_t boot_variables_end;
    paddr_t identity_paging_start;
    paddr_t identity_paging_end;
};  

#endif // BOOT_H

