
// #include <tiny/mm/bootmem.h>
// #include <tiny/mm/pmm.h>
// #include <tiny/mm/vasl.h>
// #include <tiny/mm/memory.h>

// extern struct memory_info _mem_info;

// extern struct page *mem_map;

// gdt_entry_t GDT[] = {
//     GDT_ENTRY(0ULL, 0, 0, 0),

//     // Kernel code segment entry
//     GDT_ENTRY(
//         0ULL,
//         0xFFFFF,
//         GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
//         GDT_FLAGS_64BIT 
//     ),

//     // Kernel data segment entry
//     GDT_ENTRY(
//         0ULL,
//         0xFFFFF,
//         GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
//         GDT_FLAGS_64BIT 
//     ),

//     // Userspace code segment entry
//     GDT_ENTRY(
//         0ULL,
//         0xFFFFF,
//         GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
//         GDT_FLAGS_64BIT 
//     ),

//     // Userspace data segment entry
//     GDT_ENTRY(
//         0ULL,
//         0xFFFFF,
//         GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
//         GDT_FLAGS_64BIT 
//     ),
// };

// gdt_descriptor_t GDTdescriptor = {  (sizeof(GDT) - 1), (uintptr_t)GDT  };
// extern void gdt_load(gdt_descriptor_t *gdt_descriptor, uint32_t code_segment, uint32_t data_segment);

// void init_gdt() {
//     gdt_load(&GDTdescriptor, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
// }

// // void init_memory(struct bootinfo *info) {
// //     init_gdt();

// //     clean_memory_map(&_mmap);
// //     print_mmap(_mmap);

// //     init_mem_map(_mmap, &_mem_info);
// // }


// // MEMORY PAGING

// void map_virt_to_phys(page_table_t *pml4, paddr_t phys, vaddr_t virt, uint16_t flags) {
//     flags &= 0x0FFF;
//     flags |= PAGE_FLAG_PRESENT;

//     size_t pml4_index = (virt >> 39) & 0x1ff;
//     size_t pdpt_index = (virt >> 30) & 0x1ff;
//     size_t pdt_index  = (virt >> 21) & 0x1ff;
//     size_t pt_index   = (virt >> 12) & 0x1ff;

//     if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
//         void *pdpt_address = kpalloc();

//         memset(pdpt_address, 0, sizeof(page_table_t));
//         pml4->entries[pml4_index] = ((uintptr_t)pdpt_address & PAGE_MASK) | flags;
//     }

//     page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
//     if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
//         void *pdt_address = kpalloc();
        
//         memset(pdt_address, 0, sizeof(page_table_t));
//         pdpt->entries[pdpt_index] = ((uintptr_t)pdt_address & PAGE_MASK) | flags;
//     }

//     page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
//     if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
//         void *pt_address = kpalloc();
        
//         memset(pt_address, 0, sizeof(page_table_t));
//         pdt->entries[pdt_index] = ((uintptr_t)pt_address & PAGE_MASK) | flags;
//     }

//     page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
//     pt->entries[pt_index] = (phys & PHYS_ADDR_MASK) | flags;

//     unsigned long pfn = phys >> PAGE_SHIFT;
//     mem_map[pfn]._mapcount += 1;
// }

// void unmap_virt(page_table_t *pml4, vaddr_t virt) {
//     size_t pml4_index = (virt >> 39) & 0x1ff;
//     size_t pdpt_index = (virt >> 30) & 0x1ff;
//     size_t pdt_index  = (virt >> 21) & 0x1ff;
//     size_t pt_index   = (virt >> 12) & 0x1ff;

//     if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
//         return;    
//     }

//     page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
//     if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
//         return;
//     }

//     page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
//     if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
//         return;
//     }

//     page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
//     if (!(pt->entries[pt_index] & PAGE_FLAG_PRESENT)) {
//         return;
//     }

//     pt->entries[pt_index] = 0; 
// }

// paddr_t get_phys_from_virt(page_table_t *pml4, vaddr_t virt) {
//     size_t pml4_index = (virt >> 39) & 0x1ff;
//     size_t pdpt_index = (virt >> 30) & 0x1ff;
//     size_t pdt_index  = (virt >> 21) & 0x1ff;
//     size_t pt_index   = (virt >> 12) & 0x1ff;

//     if (!(pml4->entries[pml4_index] & PAGE_FLAG_PRESENT)) {
//         return 0;    
//     }

//     page_table_t *pdpt = (page_table_t*)(pml4->entries[pml4_index] & PAGE_MASK);
//     if (!(pdpt->entries[pdpt_index] & PAGE_FLAG_PRESENT)) {
//         return 0;
//     }

//     page_table_t *pdt = (page_table_t*)(pdpt->entries[pdpt_index] & PAGE_MASK);
//     if (!(pdt->entries[pdt_index] & PAGE_FLAG_PRESENT)) {
//         return 0;
//     }

//     page_table_t *pt = (page_table_t*)(pdt->entries[pdt_index] & PAGE_MASK);
//     if (!(pt->entries[pt_index] & PAGE_FLAG_PRESENT)) {
//         return 0;
//     }

//     return (pt->entries[pt_index] & PHYS_ADDR_MASK);
// }

// void identity_map_mmap(page_table_t *pml4, struct efi_memory_map *mmap, off_t offset) {
//     // Identity map the whole mmap at an offset
//     for (size_t i = 0; i < mmap->size; ++i) {
//         struct efi_memory_descriptor *current = mmap->map + i;

//         current->virt_start = offset + current->phys_start;

//         for (size_t j = 0; j < current->npages; ++j) {
//             map_virt_to_phys(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->phys_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
//             map_virt_to_phys(pml4, (paddr_t)(current->phys_start + j * PAGE_SIZE), (vaddr_t)(current->virt_start + j * PAGE_SIZE), PAGE_FLAG_READWRITE);
//         }
//     }
// }

// // UTILS

// // This is for temporary memory allocation only!
// void *mmap_allocate_pages(size_t count, struct efi_memory_map mmap) {
//     static size_t current_mem_descriptor = 0;
//     static size_t allocated_pages        = 0;
//     static size_t remaining_pages        = 0;
    
//     // If we dont have enough pages to fulfill the request we need to change the current descriptor and find one big enough for our needs 
//     if (remaining_pages < count) {
//         for (size_t i = current_mem_descriptor; i < mmap.size; ++i) {
//             struct efi_memory_descriptor *descriptor = &mmap.map[i];

//             if (descriptor->type == EFI_CONVENTIONAL_MEMORY && descriptor->npages >= count) {
//                 current_mem_descriptor = i; 
//                 allocated_pages = 0;
//                 remaining_pages = descriptor->npages;

//                 break;
//             }
//         }
//     }

//     struct efi_memory_descriptor *descriptor = &mmap.map[current_mem_descriptor];

//     void *pages = (void*)(descriptor->phys_start + allocated_pages * PAGE_SIZE);
//     allocated_pages += count;
//     remaining_pages = descriptor->npages - allocated_pages;

//     return pages; 
// }

// void reclaim_page_table_allocations(page_table_t *pml4) {
//     // TODO
// }