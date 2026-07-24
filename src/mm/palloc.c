
#include <arch/mm/paging.h>

#include <tiny/mm/bootmem.h>
#include <tiny/mm/vasl.h>
#include <tiny/io.h>
#include <tiny/panic.h>

#include <tiny/mm/palloc.h>

static struct buddy_map maps[BUDDY_ORDER_COUNT];
struct buddy_allocator allocator = {
    .pages = NULL,
    .page_count = 0,

    .maps = maps,
    .map_count = BUDDY_ORDER_COUNT,

    .free_pages = 0
};

static paddr_t kernel_image_start = (paddr_t)0;
static paddr_t kernel_image_end   = (paddr_t)0;

struct page *page_from_pn(pn_t pn)
{
    if (pn >= allocator.page_count)
        return NULL;

    return &allocator.pages[pn];
}

static inline pn_t get_buddy_pn(pn_t pn, int order)
{
    return pn ^ ((pn_t)1 << order);
}

static inline struct list_head *get_list(struct page *page)
{
    return (struct list_head*)p_to_v(pn_to_paddr(page->pfn));
}

static void __allocator_free_block(pn_t page, int order)
{
    int previous_order = order;
    struct page *p, *buddy;

    /* 
     * Keep merging adjacent same-order buddies until we either hit the max order 
     * or we do not have any same-order adjacent buddies 
     */
    while (order < BUDDY_MAX_ORDER) {
        buddy = page_from_pn(get_buddy_pn(page, order));

        if (!buddy 
            || buddy->order != order 
            || get_bit(buddy->flags, PG_INVALID) 
            || !get_bit(buddy->flags, PG_FREE))
            break;

        list_del(get_list(buddy));
        clr_bit(buddy->flags, PG_FREE);
        allocator.maps[order].free_count--;

        /* Take the lowest-addressed buddy */
        page &= ~((pn_t)1 << order);

        /* Mark the other's order as -1, as in part of another block */
        p = page_from_pn(get_buddy_pn(page, order));
        p->order = -1;

        order++;
    }

    p = page_from_pn(page);
    p->order = order;
    list_add(get_list(p), &allocator.maps[order].list);
    allocator.maps[order].free_count++;
    set_bit(p->flags, PG_FREE);

    allocator.free_pages += ((pn_t)1 << previous_order);
}

static void __free_single_page(pn_t pg)
{
    if (pn_to_paddr(pg) >= kernel_image_start && pn_to_paddr(pg) <= kernel_image_end) {
        kprintf(KERN_ERROR, "Tried to mark a kernel page as free!\n");
        return;
    }

    __allocator_free_block(pg, 0);
}

static void split_block(int order) 
{
    struct page *p, *buddy;

    if (order > BUDDY_MAX_ORDER || order <= 0) {
        kprintf(KERN_ERROR, "Wrong order! Could not split a block of order %d!\n", order);
        return;
    }

    if (allocator.maps[order].free_count == 0) {
        split_block(order + 1);

        if (allocator.maps[order].free_count == 0)
            return;
    }

    struct free_block *b = list_take_last(&allocator.maps[order].list, struct free_block, list);
    p = page_from_pn(paddr_to_pn(v_to_p((vaddr_t)b)));

    allocator.maps[order].free_count--;
    order--;

    buddy = page_from_pn(get_buddy_pn(p->pfn, order));

    p->order = order;
    buddy->order = order;

    set_bit(p->flags, PG_FREE);
    set_bit(buddy->flags, PG_FREE);
    atomic_set(&p->count, 0);
    atomic_set(&buddy->count, 0);

    list_add(get_list(p), &allocator.maps[order].list);
    list_add(get_list(buddy), &allocator.maps[order].list);
    allocator.maps[order].free_count += 2;
}

struct page *palloc(int order, unsigned int flags)
{
    struct page *p;

    if (order > BUDDY_MAX_ORDER || order < 0)
        return NULL;

    if (allocator.maps[order].free_count == 0) {
        split_block(order + 1);

        if (allocator.maps[order].free_count == 0)
            return NULL;
    }

    struct free_block *b = list_take_last(&allocator.maps[order].list, struct free_block, list);
    p = page_from_pn(paddr_to_pn(v_to_p((vaddr_t)b)));

    allocator.maps[order].free_count--;
    clr_bit(p->flags, PG_FREE);

    if (p->pfn != (p - allocator.pages))
        panic("Something is wrong with physical page allocation!");

    p->order = order;
    allocator.free_pages -= ((pn_t)1 << order);
    atomic_set(&p->count, 1);

    if (flags & PALLOC_ZERO)
        memset(p_to_v(pn_to_paddr(p->pfn)), 0, PAGE_SIZE << order);

    return p;
}

void pfree(struct page *page)
{
    if (!page
        || page->order == -1
        || get_bit(page->flags, PG_INVALID))
        return;

    if (atomic_xadd(&page->count, -1))
        __allocator_free_block(page->pfn, page->order);
}

void *vpalloc(int order, unsigned int flags)
{
    struct page *p = palloc(order, flags);

    if (p)
        return (void *)(p_to_v(pn_to_paddr(p->pfn)));
    else 
        return NULL;
}

void vpfree(void *p)
{
    if (!ALIGNED((vaddr_t)p, PAGE_ALIGNMENT))
        return;

    struct page *page = page_from_pn(paddr_to_pn(v_to_p(p)));
    pfree(page);
}

void init_palloc(struct memory_info *info)
{
    kprintf(KERN_INFO, "Initializing buddy allocator\n");

    kernel_image_start = info->kernel_image_start;
    kernel_image_end   = info->kernel_image_end;

    allocator.page_count = info->max_pfn;
    allocator.pages = bootmem_alloc(allocator.page_count * sizeof(struct page), PAGE_ALIGNMENT);

    kprintf(KERN_INFO, "Phys pages: %d, array stored at %p\n", allocator.page_count, allocator.pages);

    memset(allocator.pages, 0, allocator.page_count * sizeof(struct page));

    for (size_t i = 0; i < allocator.page_count; ++i) {
        struct page *p = &allocator.pages[i];

        p->pfn   = i;
        p->order = -1;

        set_bit(p->flags, PG_INVALID);
    }

    for (size_t i = 0; i < allocator.map_count; ++i) {
        struct buddy_map *map = &allocator.maps[i];

        list_head_init(&map->list);
        map->free_count = 0;
    }

    for (size_t i = 0; i < info->nregions; ++i) {
        if (info->regions[i].type != REGION_TYPE_RAM)
            continue;

        paddr_t start = info->regions[i].start;
        paddr_t end   = info->regions[i].end;

        if (start == 0)
            start = 0x1000;

        for (pn_t pfn = paddr_to_pn(start); pfn < paddr_to_pn(end); ++pfn) {
            struct page *p = page_from_pn(pfn);
            clr_bit(p->flags, PG_INVALID);
            __free_single_page(pfn);
        }
    }

    for (size_t i = 0; i < BUDDY_ORDER_COUNT; ++i) {
        kprintf(KERN_DEBUG, "order: %llu, free blocks: %llu"EOL, i, allocator.maps[i].free_count);
    }

    struct page *p[10];
    for (size_t i = 0; i < 10; ++i) {
        p[i] = palloc(0, 0);
    }

    for (size_t i = 0; i < BUDDY_ORDER_COUNT; ++i) {
        kprintf(KERN_DEBUG, "order: %llu, free blocks: %llu"EOL, i, allocator.maps[i].free_count);
    }

    for (size_t i = 0; i < 10; ++i) {
        pfree(p[i]);
    }

    for (size_t i = 0; i < BUDDY_ORDER_COUNT; ++i) {
        kprintf(KERN_DEBUG, "order: %llu, free blocks: %llu"EOL, i, allocator.maps[i].free_count);
    }
}