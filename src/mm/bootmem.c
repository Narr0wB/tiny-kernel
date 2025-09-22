
#include <arch/mm/paging.h>
#include <tiny/mm/vasl.h>
#include <tiny/mm/palloc.h>
#include <tiny/io.h>
#include <tiny/panic.h>
#include <tiny/errno.h>

#include <tiny/mm/bootmem.h>

#define MAX_REGIONS 128
static struct bootmem_region __regions[MAX_REGIONS];

extern char __pkernel_start[], __pkernel_end[];
static paddr_t kernel_image_start = (paddr_t)__pkernel_start;
static paddr_t kernel_image_end = (paddr_t)__pkernel_end;

pn_t max_pfn = 0;
size_t allocable_pages = 0;

int bootmem_insert_region(paddr_t start, paddr_t end, int type) 
{
    start = PAGE_ALIGN_UP(start);
    end = PAGE_ALIGN_DOWN(end);

    // Ignore phys NULL page
    if (start == 0) {
        start = PAGE_SIZE;
        if (start >= end) 
            return 0;
    }

    pn_t end_pgn = paddr_to_pn(end);
    if (end_pgn > max_pfn) 
        max_pfn = end_pgn;

    if (start >= kernel_image_start && end <= kernel_image_end) 
        return 0;

    // In case of a region that overlaps with the kernel, split the region in two 
    // (kernel-overlapping, non kernel-overlapping) and insert the non kernel-overlapping
    if ((start < kernel_image_start && end >= kernel_image_end) 
        || (start <= kernel_image_start && end > kernel_image_end)) {

        if (start < kernel_image_start)
            bootmem_insert_region(start, kernel_image_start, type);
        
        if (end > kernel_image_end)
            bootmem_insert_region(kernel_image_end, end, type);
    }

    if (type == REGION_TYPE_RAM)
        allocable_pages += SIZE_TO_PAGES(end - start);

    kprintf(KERN_INFO, "bootmem: inserting region %p - %p\, type: %d\n", start, end, type);

    for (int i = 0; i < MAX_REGIONS; ++i) {
        if (__regions[i].start == 0) {
            __regions[i].start = start;
            __regions[i].end = end;
            __regions[i].type = type;
            return 0;
        }
    }

    kprintf(KERN_WARNING, "bootmem: could not insert region, MAX_REGIONS reached!\n");
    return -ENOMEM;
}

void *__bootmem_alloc(size_t size, unsigned long align) 
{
    // Look for a region that is capable of housing this allocation
    for (int i = 0; i < MAX_REGIONS; ++i) {
        if (__regions[i].type != REGION_TYPE_RAM || __regions[i].start == 0) 
            continue;

        paddr_t _start = ALIGN_UP(__regions[i].start, align);
        paddr_t _end   = ALIGN_DOWN(__regions[i].end, align);

        if (_start < _end && (_end - _start) > size) {
            __regions[i].start = _start + size;
            return (void *)P2V(_start);
        }
    }

    return NULL;
}

void *bootmem_alloc(size_t size, unsigned long align) 
{
    void *res = __bootmem_alloc(size, align);

    if (!res)
        panic("OOM: out of bootmem allocable memory!");
    
    return res;
}

void bootmem_get_memory_info(struct memory_info *info)
{
    info->max_pfn = max_pfn;
    info->kernel_image_start = kernel_image_start;
    info->kernel_image_end   = kernel_image_end;
    info->allocable_pages    = allocable_pages;
}

void bootmem_init_palloc() {
    for (int i = 0; i < MAX_REGIONS; ++i) {
        if (__regions[i].type != REGION_TYPE_RAM || __regions[i].start == 0)
            continue;

        for (pn_t pfn = paddr_to_pn(__regions[i].start); pfn < paddr_to_pn(__regions[i].end); ++pfn) {
            struct page *p = page_from_pn(pfn);
            clr_bit(p->flags, PG_INVALID_BIT);
            __free_single_page(pfn);
        }
    }
}