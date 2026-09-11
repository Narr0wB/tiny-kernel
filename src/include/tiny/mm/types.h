
#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H

#include <tiny/types.h>
#include <tiny/compiler.h>

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;
typedef unsigned long off_t;
typedef unsigned long pn_t;

struct bootmem_region {
    paddr_t start;
    paddr_t end;
    int type;
} __packed;

struct memory_info {
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;

    pn_t max_pfn;
    size_t allocable_pages;

    struct bootmem_region *regions;
    size_t nregions;
}; 

#endif
