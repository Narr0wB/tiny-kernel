
#include <memory/memory.h>

memory_map_t _mmap;
memory_info_t _mem_info;
__attribute__((aligned(4096))) page_table_t kernel_pml4;

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

void init_memory(memory_map_t mem_map, paddr_t kernel_start, paddr_t kernel_end) {
    init_gdt();

    _mmap = mem_map;
    _mem_info.kernel_start = kernel_start;
    _mem_info.kernel_end  = kernel_end;

    clean_memory_map(&_mmap);
    // for (int i = 0; i < _mmap.size; ++i) {
    //     memory_descriptor_t *current = (memory_descriptor_t*)((uint8_t*)_mmap.map + i * sizeof(memory_descriptor_t));
    //     kprintf("Memory segment no. [%d] type: %s || phys start: %16llx || virt start: %16llx || npages: %lld || attributes: %2lld"EOL, i, EFI_MEMORY_TYPE_STRING[current->type], current->phys_start, current->virt_start, current->npages, current->attribute);
    // }

    memset(&kernel_pml4, 0, sizeof(page_table_t));
    identity_map_mmap(&kernel_pml4, &_mmap);

    framebuffer_t *framebuffer = vga_get_current_framebuffer();
    size_t fb_size = framebuffer->len_scanline * framebuffer->width * sizeof(uint32_t);
    for (size_t i = 0; i < SIZE_TO_PAGES(fb_size); ++i) {
        map_phys_to_virt(&kernel_pml4, (paddr_t)framebuffer->base_addr + i * PAGE_SIZE, (vaddr_t)framebuffer->base_addr + i * PAGE_SIZE, PAGE_FLAG_READWRITE);
    }
    
    SWITCH_PAGE_TREE(&kernel_pml4);
}

// This function will merge memory descriptors of type EfiBootServicesData/Code with EfiConventionalMemory descriptors
void clean_memory_map(memory_map_t *mem_map) {
    // The first descriptor that classifies as "free" 
    memory_descriptor_t *current_free = NULL;
    size_t mmap_descriptors = 0;
    bool is_current_free = false;

    for (size_t i = 0; i < mem_map->size; ++i) {
        memory_descriptor_t *current = mem_map->map + i;
        
        if (current->type == EFI_CONVENTIONAL_MEMORY 
            || current->type == EFI_BOOT_SERVICES_CODE 
            || current->type == EFI_BOOT_SERVICES_DATA 
            || current->type == EFI_LOADER_CODE
            || current->type == EFI_LOADER_DATA
            || current->type == EFI_RUNTIME_SERVICES_CODE
            || current->type == EFI_RUNTIME_SERVICES_DATA) 
        {
            if (!is_current_free) {
                current_free = (mem_map->map + mmap_descriptors);
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
            *(mem_map->map + mmap_descriptors) = *current; 

            mmap_descriptors++;
        }
    }
    
    mem_map->size = mmap_descriptors;
}

// MEMORY PAGING

void map_phys_to_virt(page_table_t *pml4, paddr_t phys, vaddr_t virt, uint16_t flags) {
    flags &= 0x0FFF;
    flags |= PAGE_FLAG_PRESENT;

    size_t pml4_index = (virt >> 39) & 0x1ff;
    size_t pdpt_index = (virt >> 30) & 0x1ff;
    size_t pdt_index  = (virt >> 21) & 0x1ff;
    size_t pt_index   = (virt >> 12) & 0x1ff;

    if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
        void *pdpt_address = mmap_allocate_pages(1);

        memset(pdpt_address, 0, sizeof(page_table_t));
        pml4->entries[pml4_index] = (uintptr_t)pdpt_address | flags;
    }

    page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
    if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
        void *pdt_address = mmap_allocate_pages(1);
        
        memset(pdt_address, 0, sizeof(page_table_t));
        pdpt->entries[pdpt_index] = (uintptr_t)pdt_address | flags;
    }

    page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
    if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
        void *pt_address = mmap_allocate_pages(1);
        
        memset(pt_address, 0, sizeof(page_table_t));
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

        for (size_t j = 0; j < current->npages; ++j) {
            map_phys_to_virt(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->phys_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
        }
    }
}

// MEMORY ALLOCATION

void *mmap_allocate_pages(size_t count) {
    static size_t current_mem_descriptor = -1;
    static paddr_t free_pages            = 0;
    static size_t remaining_pages        = 0;
    
    // If we dont have enough pages to fulfill the request we need to change the current descriptor and find one big enough for our needs 
    if (remaining_pages < count) {
        for (size_t i = current_mem_descriptor + 1; i < _mmap.size; ++i) {
            memory_descriptor_t *descriptor = (_mmap.map + i);

            if (descriptor->type == EFI_CONVENTIONAL_MEMORY && descriptor->npages >= count) {
                if (descriptor->phys_start < _mem_info.kernel_end && descriptor->npages >= SIZE_TO_PAGES(_mem_info.kernel_end - _mem_info.kernel_start)) {
                    current_mem_descriptor = i;
                    free_pages = SIZE_TO_PAGES(_mem_info.kernel_end) * PAGE_SIZE; 
                    remaining_pages = descriptor->npages - SIZE_TO_PAGES(free_pages);
                    break;
                }

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
