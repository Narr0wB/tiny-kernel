
#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H

#include <types.h>
#include <sys/types.h>

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;

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

struct memory_info {
    paddr_t kernel_image_start;
    paddr_t kernel_image_end;
    paddr_t boot_variables_start;
    paddr_t boot_variables_end;
    paddr_t identity_paging_start;
    paddr_t identity_paging_end;

    paddr_t mem_map;
    size_t mem_map_pages;
    uint64_t max_pfn;
    uint64_t available_phys_pages;
}; 

#endif
