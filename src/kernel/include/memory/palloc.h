
#ifndef VALLOC_H
#define VALLOC_H

#include <common.h>

struct page_stack {
    struct page_stack *next;
};


void *page_valloc(void *hint, size_t pages, int flags);

#endif // VALLOC_H
