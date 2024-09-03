
#ifndef BOOT_H
#define BOOT_H 

#include <common.h>

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;

typedef struct framebuffer {
    void* base_addr;
    size_t size;
    
    uint32_t width;
    uint32_t height;
    uint32_t len_scanline;
} framebuffer_t;

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

typedef struct bootinfo {
    framebuffer_t framebuffer;
    memory_map_t map;
} bootinfo_t; 

#endif // BOOT_H

