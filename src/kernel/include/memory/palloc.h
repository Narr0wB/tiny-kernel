
#ifndef PALLOC_H
#define PALLOC_H

#include <common.h>
#include <memory/memory.h>
#include <memory/types.h>
#include <util/panic.h>

struct page_stack {
    struct page_stack *next;
};

void init_allocator(memory_map_t memmap);
void* kpalloc();
void kpfree(void* addr);

#endif // PALLOC_H
