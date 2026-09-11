
#include <tiny/mm/kmalloc.h>

#include <tiny/mm/vmm.h>

struct vm_area *vmap(struct vm_space *space, paddr_t phys, vaddr_t virt, size_t size, u64 flags)
{
    struct vm_area *area = (struct vm_area *)kmalloc(sizeof(struct vm_area), PAL_KERNEL);
    if (!area)
        return NULL;

    area->phys_start = phys;
    area->virt_start = virt;
    area->size       = size;
    area->flags      = flags;

    list_add(&area->list, &space->areas);
}
