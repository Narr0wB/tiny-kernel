
#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <tiny/types.h>
#include <tiny/mm/types.h>

#define REGION_TYPE_RAM  0
#define REGION_TYPE_MMIO 1

struct bootmem_region {
    paddr_t start;
    paddr_t end;
    int type;
} __packed;

int bootmem_insert_region(paddr_t start, paddr_t end, int type);
void *bootmem_alloc(size_t size, unsigned long align);
void bootmem_get_memory_info(struct memory_info *info);
void bootmem_init_palloc();

#endif // BOOTMEM_H