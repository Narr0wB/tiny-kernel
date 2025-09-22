
// #include <tiny/mm/pmm.h>
// #include <tiny/mm/memory.h>
// #include <tiny/assert.h>

// struct page_stack *_page_head = (struct page_stack *)NULL; 

// // This is a physical address to the big array of page structs describing the entire available physical memory space
// // We will use the FLAT_MEM model (i.e. for each physical frame in the boot memory map assign a struct page)
// struct page *mem_map = NULL;
// size_t max_pfn = 0;
// size_t nr_free = 0; 

// void init_mem_map(struct efi_memory_map mmap, struct memory_info *mem_info) {
//     struct efi_memory_descriptor *last_descriptor = &mmap.map[mmap.size - 1];

//     // Calculate max_pfn
//     max_pfn = (last_descriptor->phys_start >> PAGE_SHIFT) + last_descriptor->npages + 1;
//     size_t mem_map_size = max_pfn * sizeof(struct page);

//     // Allocate necessary memory for the mem_map. The continguity and the alignment (PAGE_BOUNDARY aligned) of the allocation is guaranteed by the mmap_allocate_pages function
//     mem_map = (struct page *)mmap_allocate_pages(SIZE_TO_PAGES(mem_map_size), mmap);
//     memset(mem_map, 0, SIZE_TO_PAGES(mem_map_size) * PAGE_SIZE);

//     // Make sure that the pages of the allocation do not get touched
//     for (size_t i = 0; i < SIZE_TO_PAGES(mem_map_size); ++i) {
//         uint64_t mem_map_pfn = ((paddr_t)mem_map >> PAGE_SHIFT) + i;
//         set_bit(pfn_to_page(mem_map_pfn)->flags, PG_reserved);
//     }

//     // Flag pages that are usable and pages that are not
//     struct page_stack *current = _page_head;
//     uint64_t available_pages = 0;
//     for (size_t i = 0; i < mmap.size; ++i) {
//         struct efi_memory_descriptor *descriptor = &mmap.map[i];

//         for (size_t p = 0; p < descriptor->npages; ++p) {
//             unsigned long pfn = (descriptor->phys_start >> PAGE_SHIFT) + p;

//             // If this page was already reserved, then continue
//             if (get_bit(pfn_to_page(pfn)->flags, PG_reserved)) continue;

//             if (descriptor->type == EFI_CONVENTIONAL_MEMORY) {
//                 set_bit(pfn_to_page(pfn)->flags, PG_available);

//                 // Insert this page into the linked list of available pages for the Page Frame Allocator
//                 if (_page_head == NULL) {
//                     _page_head = (struct page_stack *)(pfn << PAGE_SHIFT);
//                     current = _page_head;
//                 }
//                 else {
//                     current->next = (struct page_stack *)(pfn << PAGE_SHIFT);
//                     current = current->next;
//                     available_pages++;
//                 }
//             }
//             else if (descriptor->type == EFI_LOADER_CODE) {
//                 set_bit(pfn_to_page(pfn)->flags, PG_reserved);
//             }
//             else  {
//                 set_bit(pfn_to_page(pfn)->flags, PG_mmio); // TODO: not sure about this
//             }
//         }
//     }

//     mem_info->max_pfn = max_pfn;
//     mem_info->available_phys_pages = available_pages;
//     mem_info->mem_map = (paddr_t)mem_map;
//     mem_info->mem_map_pages = SIZE_TO_PAGES(mem_map_size);

//     nr_free = available_pages;
// }

// void* kpalloc() {
//     kassert(_page_head != NULL);
//     nr_free--;

//     struct page_stack *current = _page_head;
//     _page_head = _page_head->next;
//     return (void*)current;
// }

// void kpfree(void* addr) {
//     kassert(IS_PAGE_ALIGNED((paddr_t)addr));
//     nr_free++;

//     struct page_stack *current = _page_head;
//     _page_head = (struct page_stack *)addr;
//     _page_head->next = current;
// }
