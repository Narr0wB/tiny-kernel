
#include <memory/palloc.h>

struct page_stack *_page_head = NULL; 

void init_allocator(memory_map_t memmap) {
    struct page_stack *tmp = NULL;

    for (size_t i = 0; i < memmap.size; ++i) {
        memory_descriptor_t *current = &memmap.map[i];

        if (current->type != EFI_CONVENTIONAL_MEMORY) continue;

        for (size_t j = 0; j < current->npages; ++j) {
            if (_page_head == NULL) {
                _page_head = (struct page_stack *)current->phys_start;
                tmp = _page_head;
            } 
            else {
                tmp->next = (struct page_stack *)current->phys_start; 
                tmp = tmp->next;
            }
        }
    } 
}

void* kpalloc() {
    kassert(_page_head != NULL);

    struct page_stack *current = _page_head;
    _page_head = _page_head->next;

    return (void*)current;
}

void kpfree(void* addr) {
    kassert(IS_PAGE_ALIGNED((paddr_t)addr));

    struct page_stack *tmp = _page_head;
    _page_head = (struct page_stack *)addr;
    _page_head->next = tmp;
}
