
#ifndef PALLOC_H
#define PALLOC_H

#include <types.h>
#include <memory/types.h>

#define PG_locked   0
#define PG_available 1
#define PG_uptodate 2 
#define PG_dirty    3
#define PG_reserved 4
#define PG_private  5
#define PG_mmio     6

#define pfn_to_page(pfn)  (mem_map + (pfn))
#define pfn_to_addr(pfn)  (pfn << PAGE_SHIFT)
#define page_to_pfn(page) ((unsigned long)((page) - mem_map))
#define addr_to_pfn(addr) (addr >> PAGE_SHIFT)

struct page {
    unsigned long flags;
    atomic_t _mapcount; // Count of how many virtual mappings to this page are there
};

struct page_stack {
    struct page_stack *next;
};

void init_mem_map(memory_map_t mmap, struct memory_info *mem_info);

void init_phys_allocator(memory_map_t mmap, struct memory_info *meminfo);
void* kpalloc();
void kpfree(void* addr);

#endif // PALLOC_H
