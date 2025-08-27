
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define SIZE_TO_PAGES(size) \
    (size_t)(size + 0x1000 - 1)/0x1000

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;

struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
};

typedef struct memory_descriptor {
    uint32_t    type;
    uint32_t    pad;
    paddr_t     phys_start;
    vaddr_t     virt_start;
    uint64_t    npages;
    uint64_t    attribute;
    uint64_t    padding;
} memory_descriptor_t;

typedef struct memory_map {
    memory_descriptor_t *map;
    size_t size;
} memory_map_t;

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
