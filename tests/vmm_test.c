/* Run: cc -Isrc/include -Isrc/arch/include tests/vmm_test.c src/mm/vmm.c -o /tmp/vmm_test && /tmp/vmm_test */
#include <assert.h>
#include <tiny/mm/vmm.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/mm/palloc.h>
#include <tiny/mm/vasl.h>
#include <tiny/errno.h>

int vm_page_alloc(struct vm_pt_page *pg, void *ctx);

static struct vm_area area;
static struct page page = {.pfn = 0x123};
static pgd_t root;
static int map_calls;
static bool fail_area, fail_page, fail_map;
static int free_calls;

void kfree(void *p)
{
    assert(p == &area);
    ++free_calls;
}

void *kmalloc(size_t size, unsigned int flags)
{
    assert(size == sizeof(area) && flags == PAL_KERNEL);
    return fail_area ? NULL : &area;
}

struct page *palloc(int order, unsigned int flags)
{
    assert(order == 0 && flags == PALLOC_ZERO);
    return fail_page ? NULL : &page;
}

int vm_map(pgd_t *pgd, paddr_t phys, vaddr_t virt, size_t size,
            u64 flags, vm_pt_alloc_t allocator)
{
    assert(pgd == &root && phys == 0x4000 && virt == 0x8000);
    assert(size == PAGE_SIZE && flags == PAGE_FLAG_READWRITE);
    assert(allocator == vm_page_alloc);
    if (fail_map)
        return -ENOMEM;
    struct vm_pt_page pg;
    assert(allocator(&pg, NULL) == 0);
    assert(pg.phys == 0x123000);
    assert(pg.virt == (void *)p_to_v(pg.phys));
    ++map_calls;
    return 0;
}

int main(void)
{
    struct vm_space space = {.pgd = &root};
    list_head_init(&space.areas);
    assert(vmap(&space, 0x4000, 0x8000, PAGE_SIZE,
                PAGE_FLAG_READWRITE) == &area);
    assert(map_calls == 1 && space.areas.next == &area.list);
    assert(space.areas.prev == &area.list);
    assert(area.phys_start == 0x4000 && area.virt_start == 0x8000);
    assert(area.size == PAGE_SIZE && area.flags == PAGE_FLAG_READWRITE);

    fail_area = true;
    assert(vmap(&space, 0, 0, PAGE_SIZE, 0) == NULL);
    assert(map_calls == 1 && space.areas.next == &area.list);

    fail_area = false;
    fail_map = true;
    assert(vmap(&space, 0x4000, 0x8000, PAGE_SIZE, PAGE_FLAG_READWRITE) == NULL);
    assert(free_calls == 1 && map_calls == 1);

    fail_page = true;
    struct vm_pt_page pg;
    assert(vm_page_alloc(&pg, NULL) == -ENOMEM);
    return 0;
}
