
#include <arch/x86/mm/paging.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/mm/palloc.h>
#include <tiny/mm/vasl.h>
#include <tiny/errno.h>

#include <tiny/mm/vmm.h>

extern pgd_t pgd;
struct vm_space kernel_space;


int vm_page_alloc(struct vm_pt_page *pg, void *ctx)
{
    struct page *page = palloc(0, PALLOC_ZERO);
    if (!page)
        return -ENOMEM;

    pg->phys = pn_to_paddr(page->pfn);
    pg->virt = (void *)p_to_v(pg->phys);

    return 0;
}


struct vm_area *vmap(struct vm_space *space, paddr_t phys, vaddr_t virt, size_t size, u64 flags)
{
    struct vm_area *area = (struct vm_area *)kmalloc(sizeof(struct vm_area), PAL_KERNEL);
    if (!area)
        return NULL;

    area->phys_start = phys;
    area->virt_start = virt;
    area->size       = size;
    area->flags      = flags;

    if (vm_map(space->pgd, phys, virt, size, flags, vm_page_alloc)) {
        kfree(area);
        return NULL;
    }

    list_add(&area->list, &space->areas);
    return area;
}


void init_vmm()
{
    kernel_space.pgd = &pgd;
    list_head_init(&kernel_space.areas);
}