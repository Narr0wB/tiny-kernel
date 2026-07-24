
#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <tiny/types.h>
#include <tiny/mm/types.h>
#include <tiny/boot/boot.h>

#define REGION_TYPE_RAM   0
#define REGION_TYPE_MMIO  1
#define REGION_TYPE_ALLOC 2

void bootmem_init(struct bootinfo *info);
int bootmem_insert_region(paddr_t start, paddr_t end, int type);
void *bootmem_alloc(size_t size, unsigned long align);
void bootmem_get_memory_info(struct memory_info *info);

#endif // BOOTMEM_H