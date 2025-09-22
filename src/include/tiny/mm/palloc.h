
#ifndef PALLOC_H
#define PALLOC_H

#include <arch/atomic.h>
#include <tiny/types.h>
#include <tiny/list.h>
#include <tiny/mm/types.h>

/* Page flags */
#define PG_INVALID_BIT 0

struct page {
    atomic_t count;
    pn_t pnumber;

    int order;
    struct list_head list;

    uint32_t flags;
    vaddr_t virt;
};

#define BUDDY_ORDERS 6

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
void __free_single_page(pn_t pg);

struct page *palloc(int order, unsigned int flags);
void pfree(struct page *p, int order);

void *vpalloc(int order, unsigned int flags);
void vpfree(void *p, int order);

#endif // PALLOC_H