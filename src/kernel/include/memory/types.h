
#ifndef MEMORY_TYPES_H
#define MEMORY_TYPES_H

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

#endif
