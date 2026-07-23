
#include <arch/mm/paging.h>
#include <tiny/mm/bootmem.h>
#include <tiny/mm/vasl.h>
#include <tiny/io.h>
#include <tiny/panic.h>

#include <tiny/mm/palloc.h>

static struct buddy_map maps[BUDDY_ORDERS];
struct buddy_allocator allocator = {
    .pages = NULL,
    .page_count = 0,

    .maps = maps,
    .map_count = BUDDY_ORDERS,

    .free_pages = 0
};

static paddr_t kernel_image_start = (paddr_t)0;
static paddr_t kernel_image_end   = (paddr_t)0;

void init_palloc(struct memory_info *info)
{
    kprintf(KERN_INFO, "Initializing buddy allocator\n");

    kernel_image_start = info->kernel_image_start;
    kernel_image_end   = info->kernel_image_end;

    allocator.page_count = info->max_pfn;
    allocator.pages = bootmem_alloc(allocator.page_count * sizeof(struct page), PAGE_ALIGNMENT);

    kprintf(KERN_INFO, "Phys pages: %d, array stored at %p\n", allocator.page_count, allocator.pages);

    memset(allocator.pages, 0, allocator.page_count * sizeof(struct page));

    /* Init all pages */
    for (size_t i = 0; i < allocator.page_count; ++i) {
        struct page *p = &allocator.pages[i];

        p->pnumber = i;
        p->order   = -1;
        p->virt    = p_to_v(p->pnumber << PAGE_SHIFT);

        set_bit(p->flags, PG_INVALID_BIT);
    }

    /* Init all maps */
    for (size_t i = 0; i < allocator.map_count; ++i) {
        struct buddy_map *map = &allocator.maps[i];

        list_head_init(&map->list);
        map->free_count = 0;
    }
}

struct page *page_from_pn(pn_t pn)
{
    if (pn > allocator.page_count - 1)
        return NULL;

    return &allocator.pages[pn];
}

static inline pn_t get_buddy_pn(pn_t pn, int order)
{
    return pn ^ (1 << order);
}

static void __allocator_free_block(pn_t page, int order)
{
    int previous_order = order;
    struct page *p, *buddy;

    /* 
     * Keep merging adjacent same-order buddies until we either hit the max order 
     * or we do not have any same-order adjacent buddies 
     */
    while (order < BUDDY_ORDERS - 1) {
        buddy = page_from_pn(get_buddy_pn(page, order));

        if (buddy->order != order || get_bit(buddy->flags, PG_INVALID_BIT))
            break;

        list_del(&buddy->list);
        allocator.maps[order].free_count--;

        /* Take the lowest-addressed buddy */
        page &= ~(1 << order);

        /* Mark the other's order as -1, as in part of another block */
        p = page_from_pn(get_buddy_pn(page, order));
        p->order = -1;

        order++;
    }

    p = page_from_pn(page);
    p->order = order;
    list_add(&p->list, &allocator.maps[order].list);
    allocator.maps[order].free_count++;

    allocator.free_pages += 1 << previous_order;
}

void __free_single_page(pn_t pg)
{
    if (pn_to_paddr(pg) > kernel_image_start && pn_to_paddr(pg) < kernel_image_end) {
        kprintf(KERN_ERROR, "Tried to mark a kernel page as free!\n");
        return;
    }

    __allocator_free_block(pg, 0);
}

void pfree(struct page *page, int order)
{
    if (!page) {
        kprintf(KERN_ERROR, "Null page!\n");
        return;
    }

    __allocator_free_block(page->pnumber, order);
}

static void split_block(int order) 
{
    struct page *p, *buddy;

    if (order >= BUDDY_ORDERS || order < 0) {
        kprintf(KERN_ERROR, "Wrong order! Could not split a block of order %d!\n", order);
        return;
    }

    if (allocator.maps[order].free_count == 0) {
        split_block(order + 1);

        if (allocator.maps[order].free_count == 0)
            return;
    }

    p = list_take_last(&allocator.maps[order].list, struct page, list);

    allocator.maps[order].free_count--;
    order--;

    buddy = page_from_pn(get_buddy_pn(p->pnumber, order));
    p->order = order;
    buddy->order = order;

    list_add(&p->list, &allocator.maps[order].list);
    list_add(&buddy->list, &allocator.maps[order].list);
    allocator.maps[order].free_count += 2;
}

struct page *palloc(int order, unsigned int flags)
{
    struct page *p;

    if (allocator.maps[order].free_count == 0) {
        split_block(order + 1);

        if (allocator.maps[order].free_count == 0)
            return NULL;
    }

    p = list_take_last(&allocator.maps[order].list, struct page, list);
    allocator.maps[order].free_count--;

    if (p->pnumber != (p - allocator.pages))
        panic("Something is wrong with physical page allocation!");
    
    p->order = -1;
    allocator.free_pages -= 1 << order;

    if (p)
        atomic_inc(&p->count);

    return p;
}

void *vpalloc(int order, unsigned int flags)
{
    struct page *p = palloc(order, flags);

    if (p)
        return (void *)(p->virt);
    else 
        return NULL;
}

void vpfree(void *p, int order)
{
    struct page *page = page_from_pn(paddr_to_pn(v_to_p(p)));
    pfree(page, order);
}