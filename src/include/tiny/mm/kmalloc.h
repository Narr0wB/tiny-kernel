
#ifndef KMALLOC_H
#define KMALLOC_H

#include <tiny/types.h>
#include <tiny/mm/types.h>

#define SLAB_FRAME_BLOCK_SIZE 3 /* A single slab frame is 2^3 pages, or 8 * 4Kib = 32KiB */
#define SLAB_POISON 0xCAFEBABE

#define PAL_KERNEL (0)

struct slab_frame_free_list {
    struct slab_frame_free_list *next;
};

struct slab_frame {
    struct slab_frame *next;
    void *start_addr;
    void *end_addr;
    int obj_count;
    struct slab_frame_free_list *free_list;
    int free_count;
};

struct slab_allocator {
    const char *name;
    size_t obj_size;
    struct slab_frame *head;
    int frame_count;
};

void *kmalloc(size_t size, unsigned int flags);
void kfree(void *p);

#endif // KMALLOC_H