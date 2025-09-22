
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
} struct efi_memory_descriptor;

typedef struct efi_memory_map {
    struct efi_memory_descriptor *map;
    size_t size;
} struct efi_memory_map;

struct bootinfo {
    struct framebuffer framebuffer;
    struct efi_memory_map map;
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;
    paddr_t boot_vars_start;
    paddr_t boot_vars_end;
}; 

#endif // BOOT_H
