
#ifndef BOOT_H
#define BOOT_H 

#include <tiny/types.h>
#include <tiny/mm/types.h>

struct efi_memory_descriptor {
    uint32_t    type;
    uint32_t    pad;
    paddr_t     phys_start;
    vaddr_t     virt_start;
    uint64_t    npages;
    uint64_t    attribute;
    uint64_t    padding;
};

struct efi_memory_map {
    struct efi_memory_descriptor *map;
    size_t size;
};

struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
};

struct bootinfo {
    pn_t                  max_pfn;
    struct framebuffer    framebuffer;
    struct efi_memory_map map;
    paddr_t               kernel_image_start;
    paddr_t               kernel_image_end;
    paddr_t               kernel_stack_start;
    paddr_t               kernel_stack_end;
    paddr_t               rsdp;
} __packed;

#endif // BOOT_H

