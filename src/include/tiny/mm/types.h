
#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H

#include <tiny/types.h>

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;
typedef unsigned long off_t;
typedef unsigned long pn_t;

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

struct memory_info {
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;

    pn_t max_pfn;
    size_t allocable_pages;
}; 

#endif
