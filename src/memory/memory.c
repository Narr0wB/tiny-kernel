
#include <memory/memory.h>
#include <memory/pmm.h>
#include <memory/vasl.h>

memory_map_t _mmap;
struct memory_info _mem_info;
page_table_t *kernel_pml4;

extern struct page *mem_map;

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

void init_gdt() {
    gdt_load(&GDTdescriptor, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
}

void init_memory(struct bootinfo *info) {
    init_gdt();

    _mmap = info->map;

    _mem_info.kernel_image_start    = info->kernel_image_start;
    _mem_info.kernel_image_end      = info->kernel_image_end;
    _mem_info.boot_variables_start  = info->boot_variables_start;
    _mem_info.boot_variables_end    = info->boot_variables_end;
    _mem_info.identity_paging_start = info->identity_paging_start;
    _mem_info.identity_paging_end   = info->identity_paging_end;
    _mem_info.max_pfn               = 0;

    clean_memory_map(&_mmap);
    print_mmap(_mmap);

    init_mem_map(_mmap, &_mem_info);
}

// This function will merge memory descriptors of type EfiBootServicesData/Code with EfiConventionalMemory descriptors
void clean_memory_map(memory_map_t *mem_map) {
    // The first descriptor that classifies as "free" 
    memory_descriptor_t *current_free = NULL;
    size_t mmap_descriptors = 0;
    bool is_current_free = false;

    for (size_t i = 0; i < mem_map->size; ++i) {
        memory_descriptor_t *current = &mem_map->map[i];
        
        // If there are any memory segements between the start of the kernel image and the end of the identity paging then its kernel memory, do not touch.
        if (current->phys_start < _mem_info.identity_paging_end && current->phys_start > _mem_info.kernel_image_start) {
            current->type = EFI_LOADER_CODE;

            is_current_free = false;
            mem_map->map[mmap_descriptors] = *current; 
            mmap_descriptors++;

            continue;
        };

        // Ignore, residuals from firmware
        if (current->type == EFI_RESERVED_MEMORY_TYPE) continue; 
        
        if (current->type == EFI_CONVENTIONAL_MEMORY 
            || current->type == EFI_BOOT_SERVICES_CODE 
            || current->type == EFI_BOOT_SERVICES_DATA 
            || current->type == EFI_RUNTIME_SERVICES_CODE
            || current->type == EFI_RUNTIME_SERVICES_DATA) 
        {
            if (is_current_free && current->phys_start != current_free->phys_start + current_free->npages * PAGE_SIZE) {
                is_current_free = false;
            }


            if (!is_current_free) {
                current_free = &mem_map->map[mmap_descriptors];
                *current_free = *current;
                current_free->type = EFI_CONVENTIONAL_MEMORY;

                is_current_free = true;

                mmap_descriptors++;
                continue;
            }
            else {
                current_free->npages += current->npages;
            }
        }
        else {
            is_current_free = false;
            mem_map->map[mmap_descriptors] = *current; 
            mmap_descriptors++;
        }
    }
    
    mem_map->size = mmap_descriptors;
}

// MEMORY PAGING

void map_virt_to_phys(page_table_t *pml4, paddr_t phys, vaddr_t virt, uint16_t flags) {
    flags &= 0x0FFF;
    flags |= PAGE_FLAG_PRESENT;

    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        void *pdpt_address = kpalloc();

        memset(pdpt_address, 0, sizeof(page_table_t));
        pml4->entries[pml4_index] = ((uintptr_t)pdpt_address & PAGE_MASK) | flags;
    }

    page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        void *pdt_address = kpalloc();
        
        memset(pdt_address, 0, sizeof(page_table_t));
        pdpt->entries[pdpt_index] = ((uintptr_t)pdt_address & PAGE_MASK) | flags;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        void *pt_address = kpalloc();
        
        memset(pt_address, 0, sizeof(page_table_t));
        pdt->entries[pdt_index] = ((uintptr_t)pt_address & PAGE_MASK) | flags;
    }

    page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
    pt->entries[pt_index] = (phys & PHYS_ADDR_MASK) | flags;

    unsigned long pfn = phys >> PAGE_SHIFT;
    mem_map[pfn]._mapcount += 1;
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

void identity_map_mmap(page_table_t *pml4, memory_map_t *mmap, off_t offset) {
    // Identity map the whole mmap at an offset
    for (size_t i = 0; i < mmap->size; ++i) {
        memory_descriptor_t *current = mmap->map + i;

        current->virt_start = offset + current->phys_start;

        for (size_t j = 0; j < current->npages; ++j) {
            map_virt_to_phys(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->phys_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
            map_virt_to_phys(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->virt_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
        }
    }
}

// UTILS

// This is for temporary memory allocation only!
void *mmap_allocate_pages(size_t count, memory_map_t mmap) {
    static size_t current_mem_descriptor = 0;
    static size_t allocated_pages        = 0;
    static size_t remaining_pages        = 0;
    
    // If we dont have enough pages to fulfill the request we need to change the current descriptor and find one big enough for our needs 
    if (remaining_pages < count) {
        for (size_t i = current_mem_descriptor; i < mmap.size; ++i) {
            memory_descriptor_t *descriptor = &mmap.map[i];

            if (descriptor->type == EFI_CONVENTIONAL_MEMORY && descriptor->npages >= count) {
                current_mem_descriptor = i; 
                allocated_pages = 0;
                remaining_pages = descriptor->npages;

                break;
            }
        }
    }

    memory_descriptor_t *descriptor = &mmap.map[current_mem_descriptor];

    void *pages = (void*)(descriptor->phys_start + allocated_pages * PAGE_SIZE);
    allocated_pages += count;
    remaining_pages = descriptor->npages - allocated_pages;

    return pages; 
}


void print_mmap(memory_map_t mmap) {
    kprintf(EOL);
    for (size_t i = 0; i < mmap.size; ++i) {
        memory_descriptor_t *current = &mmap.map[i];
        kprintf("[DEBUG] DESC. NO: %d TYPE: %d PHYS_START: %p NPAGES: %d"EOL, i+1, current->type, current->phys_start, current->npages);
    }
}

void reclaim_page_table_allocations(page_table_t *pml4) {
    // TODO
}