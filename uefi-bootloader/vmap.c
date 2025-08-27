
#include <vmap.h>
#include <efi.h>
#include <efilib.h>

paddr_t last_allocated_addr = 0;
uint32_t allocations = 0;

void map_virt_to_phys(page_table_t *pml4, paddr_t phys, vaddr_t virt, uint16_t flags) {
    flags &= 0x0FFF;
    flags |= PAGE_FLAG_PRESENT;

    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, 1, &last_allocated_addr);
        void *pdpt_address = (void *)last_allocated_addr;
        last_allocated_addr += PAGE_SIZE;
        allocations++;

        ZeroMem(pdpt_address, sizeof(page_table_t));
        pml4->entries[pml4_index] = (uintptr_t)pdpt_address | flags;
    }

    page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, 1, &last_allocated_addr);
        void *pdt_address = (void *)last_allocated_addr;
        last_allocated_addr += PAGE_SIZE;
        allocations++;
        
        ZeroMem(pdt_address, sizeof(page_table_t));
        pdpt->entries[pdpt_index] = (uintptr_t)pdt_address | flags;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, 1, &last_allocated_addr);
        void *pt_address = (void *)last_allocated_addr; 
        last_allocated_addr += PAGE_SIZE;
        allocations++;
        
        ZeroMem(pt_address, sizeof(page_table_t));
        pdt->entries[pdt_index] = (uintptr_t)pt_address | flags;
    }

    page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
    pt->entries[pt_index] = (phys & PHYS_ADDR_MASK) | flags;
}

void unmap_virt(page_table_t *pml4, vaddr_t virt) {
    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        return;    
    }

    page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        return;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        return;
    }

    page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
    if (!(pt->entries[pt_index] & PAGE_FLAG_PRESENT)) {
        return;
    }

    pt->entries[pt_index] = 0; 
}

paddr_t get_phys_from_virt(page_table_t *pml4, vaddr_t virt) {
    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        return 0;    
    }

    page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
    if (!(pt->entries[pt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    return (pt->entries[pt_index] & PHYS_ADDR_MASK);
}

void identity_map_mmap(page_table_t *pml4, memory_map_t *mmap) {
    for (size_t i = 0; i < mmap->size; ++i) {
        memory_descriptor_t *current = mmap->map + i;

        if (current->type == 0) continue;

        for (size_t j = 0; j < current->npages; ++j) {
            map_virt_to_phys(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->phys_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
        }
    }
}