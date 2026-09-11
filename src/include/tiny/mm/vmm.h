
#ifndef VMM_H
#define VMM_H

#ifdef ARCH_x86
#include <arch/x86/mm/paging.h>
#endif 

#include <tiny/types.h>
#include <tiny/list.h>

struct vm_space {
    pgd_t *pgd; 
    struct list_head areas;
};

struct vm_area {
    vaddr_t          virt_start;
    paddr_t          phys_start;
    size_t           size;
    u64              flags;
    struct list_head list;
};

struct vm_area *vmap(struct vm_space *space, paddr_t phys, vaddr_t virt, size_t size, u64 flags);
void vunmap(struct vm_area *area);

#endif // VMM_H
