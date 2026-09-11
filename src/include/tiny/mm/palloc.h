
#ifndef PALLOC_H
#define PALLOC_H

#include <arch/x86/atomic.h>
#include <tiny/types.h>
#include <tiny/list.h>
#include <tiny/mm/types.h>

/* 
 * Page flags 
 */
#define PG_FREE    0
#define PG_INVALID 1

/* 
 * Palloc flags 
 */
#define PALLOC_ZERO (1U << 0)


#define BUDDY_ORDER_COUNT 9
#define BUDDY_MAX_ORDER (BUDDY_ORDER_COUNT - 1)

struct page {
    atomic_t count;
    int8_t order;
    uint8_t flags;
    pn_t pfn;
};

struct free_block {
    struct list_head list;
};

struct buddy_map {
    struct list_head list;
    int free_count;
};

struct buddy_allocator {
    struct page *pages;
    size_t page_count;

    struct buddy_map *maps;
    size_t map_count;

    size_t free_pages;
};

struct page *page_from_pn(pn_t pn);

void init_palloc(struct memory_info *info);

struct page *palloc(int order, unsigned int flags);
void pfree(struct page *p);

void *vpalloc(int order, unsigned int flags);
void vpfree(void *p);

#endif // PALLOC_H
