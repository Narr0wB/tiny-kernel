
#include "video/video.h"
#include <memory/memory.h>

memory_map_t _mmap;
__attribute__((aligned(4096))) page_table_t pml4;

gdt_entry_t GDT[] = {
    GDT_ENTRY(0ULL, 0, 0, 0),

    // Kernel code segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
        GDT_FLAGS_64BIT 
    ),

    // Kernel data segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
        GDT_FLAGS_64BIT 
    ),

    // Userspace code segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
        GDT_FLAGS_64BIT 
    ),

    // Userspace data segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
        GDT_FLAGS_64BIT 
    ),
};

gdt_descriptor_t GDTdescriptor = {  (sizeof(GDT) - 1), (uintptr_t)GDT  };
extern void gdt_load(gdt_descriptor_t *gdt_descriptor, uint32_t code_segment, uint32_t data_segment);
extern void load_cr3(page_table_t *pml4);

void init_gdt() {
    gdt_load(&GDTdescriptor, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
}

void init_memory(memory_map_t mem_map) {
    _mmap = mem_map;
    init_gdt();

    memset(&pml4, 0, sizeof(page_table_t));
    // identity_map_mmap(_mmap);

    // load_cr3(&pml4);
}

// MEMORY PAGING

void map_virt_to_phys(vaddr_t virt, paddr_t phys, uint16_t flags) {
    flags &= 0x0FFF;
    flags |= PAGE_FLAG_PRESENT;

    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4.entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        void *pdpt_address = mmap_allocate_pages(1);

        memset(pdpt_address, 0, sizeof(page_table_t));
        pml4.entries[pml4_index] = (uintptr_t)pdpt_address | flags;
    }

    page_table_t *pdpt = (page_table_t*)(pml4.entries[pml4_index] & PAGE_ALIGN);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        void *pdt_address = mmap_allocate_pages(1);
        
        memset(pdt_address, 0, sizeof(page_table_t));
        pdpt->entries[pdpt_index] = (uintptr_t)pdt_address | flags;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_ALIGN);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        void *pt_address = mmap_allocate_pages(1);
        
        memset(pt_address, 0, sizeof(page_table_t));
        pdt->entries[pdt_index] = (uintptr_t)pt_address | flags;
    }

    page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_ALIGN);
    pt->entries[pt_index] = (phys & PHYS_ADDR_MASK) | flags;
}

paddr_t get_phys_from_virt(vaddr_t virt) {
    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4.entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        return 0;    
    }

    page_table_t *pdpt = (page_table_t*)pml4.entries[pml4_index];
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    page_table_t *pdt = (page_table_t*)pdpt->entries[pdpt_index];
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    page_table_t *pt = (page_table_t*)pdt->entries[pdt_index];
    if (!(pt->entries[pt_index] & PAGE_FLAG_PRESENT)) {
        return 0;
    }

    return (pt->entries[pt_index] & PHYS_ADDR_MASK);
}

void identity_map_mmap(memory_map_t mmap) {
    for (size_t i = 0; i < mmap.size; ++i) {
        memory_descriptor_t *current = mmap.map + i;

        for (size_t j = 0; j < current->npages; ++j) {
            map_virt_to_phys((vaddr_t)(current->phys_start + j * PAGE_SIZE), (paddr_t)(current->phys_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
        }
    }
}

// MEMORY ALLOCATION
void *mmap_allocate_pages(size_t count) {
    static size_t current_mem_descriptor = 10;
    static paddr_t free_pages            = 0;
    static size_t remaining_pages        = 0;

    // If we dont have enough pages to fulfill the request we need to change the current descriptor and find one big enough for our needs 
    if (remaining_pages < count) {
        for (size_t i = current_mem_descriptor + 1; i < _mmap.size; ++i) {
            memory_descriptor_t *descriptor = (_mmap.map + i);

            if (descriptor->type == EFI_CONVENTIONAL_MEMORY && descriptor->npages >= count) {
                current_mem_descriptor = i; 
                free_pages = descriptor->phys_start;
                remaining_pages = descriptor->npages;

                break;
            }
        }
    }

    void *pages = (void*)(free_pages);
    remaining_pages -= count;
    free_pages += count * PAGE_SIZE;

    return pages; 
}
