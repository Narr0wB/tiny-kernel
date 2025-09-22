
#include <arch/mm/paging.h>
#include <tiny/mm/palloc.h>
#include <tiny/kernel.h>
#include <tiny/io.h>

#include <tiny/mm/kmalloc.h>

static struct slab_allocator slabs[] = {
    { .name = "kmalloc_32", .obj_size = 32, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_64", .obj_size = 64, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_128", .obj_size = 128, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_256", .obj_size = 256, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_512", .obj_size = 512, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_1024", .obj_size = 1024, .head = NULL, .frame_count = 0 },
    { .name = "kmalloc_2048", .obj_size = 2048, .head = NULL, .frame_count = 0 },
};

struct slab_frame *slab_alloc_new_frame(struct slab_allocator *alloc)
{
    struct slab_frame *new_frame = (struct slab_frame *)vpalloc(SLAB_FRAME_BLOCK_SIZE, 0);
    memset(new_frame, 0, PAGE_SIZE * (1 << SLAB_FRAME_BLOCK_SIZE));
    
    new_frame->start_addr = (void *)ALIGN_UP(((vaddr_t)(new_frame) + sizeof(struct slab_frame)), alloc->obj_size);
    new_frame->obj_count = (((1 << SLAB_FRAME_BLOCK_SIZE) * PAGE_SIZE) - sizeof(struct slab_frame)) / alloc->obj_size;
    new_frame->end_addr = (void *)((uintptr_t)new_frame->start_addr + new_frame->obj_count * alloc->obj_size);
    new_frame->free_count = new_frame->obj_count;

    struct slab_frame_free_list **curr_obj = &new_frame->free_list;
    for (int i = 0; i < new_frame->obj_count; ++i, curr_obj = &((*curr_obj)->next)) {
        void *obj_addr = (void *)((uintptr_t)new_frame->start_addr + (i * alloc->obj_size));

        for (int j = 0; j < alloc->obj_size / 4; j++) 
            ((uint32_t *)obj_addr)[j] = SLAB_POISON;

        *curr_obj = (struct slab_frame_free_list *)obj_addr;
    }

    struct slab_frame **curr_frame = &alloc->head;
    for (; *curr_frame != NULL; curr_frame = &(*curr_frame)->next)
        ;;

    *curr_frame = new_frame;
    alloc->frame_count++;
    return new_frame;
}

void slab_free_frame(struct slab_allocator *alloc, struct slab_frame *frame)
{
    struct slab_frame **ptr = &alloc->head;

    for (int i = 0; *ptr != frame && i < alloc->frame_count; ptr = &(*ptr)->next, ++i)
        ;;

    *ptr = frame->next;
    alloc->frame_count--;
    vpfree((void *)frame, SLAB_FRAME_BLOCK_SIZE);
}

void *slab_alloc_obj(struct slab_allocator *alloc, struct slab_frame *frame)
{
    struct slab_frame_free_list *obj, *next;

  try_again:
    if (!frame->free_list)
        return NULL;

    obj = frame->free_list;

    uint32_t *poison = (uint32_t *)(obj + 1);
    int poison_cnt = (alloc->obj_size - sizeof(struct slab_frame_free_list *)) / 4;
    for (int i = 0; i < poison_cnt; ++i) {
        if (poison[i] != SLAB_POISON) {
            kprintf(KERN_ERROR, "Found an invalid entry in the slab allocator! Skipping it...\n");

            frame->free_list = frame->free_list->next;
            goto try_again;
        }
    }

    frame->free_list = frame->free_list->next;
    frame->free_count--;

    return (void *)obj;
}

void slab_free_obj(struct slab_allocator *alloc, struct slab_frame *frame, void *obj)
{
    struct slab_frame_free_list *new = obj, *old;

    uint32_t *slot = (uint32_t *)obj;
    for (int i = 0; i < alloc->obj_size / 4; ++i)
        slot[i] = SLAB_POISON;

    new->next = frame->free_list;
    frame->free_list = new;
    frame->free_count++;

    /* If the frame is empty, then free it */
    if (frame->free_count == frame->obj_count)
        slab_free_frame(alloc, frame);
}

bool slab_has_addr(struct slab_allocator *alloc, void *addr)
{
    for (struct slab_frame *frame = alloc->head; frame; frame = frame->next) {
        if (addr >= frame->start_addr && addr < frame->end_addr)
            return true;
    }

    return false;
}

void *slab_malloc(struct slab_allocator *alloc)
{
    for (struct slab_frame *frame = alloc->head; frame; frame = frame->next) {
        if (frame->free_count > 0)
            return slab_alloc_obj(alloc, frame);
    }

    /* The new frame is already added to the list by the slab_alloc_new_frame() function */
    struct slab_frame *new_frame = slab_alloc_new_frame(alloc);
    if (!new_frame)
        return NULL;

    return slab_alloc_obj(alloc, new_frame);
}

void slab_free(struct slab_allocator *alloc, void *obj)
{
    for (struct slab_frame *frame = alloc->head; frame; frame = frame->next)
        if (obj >= frame->start_addr && obj < frame->end_addr)
            slab_free_obj(alloc, frame, obj);
}

void slab_clear(struct slab_allocator *alloc)
{
    struct slab_frame *frame, *next;
    for (frame = alloc->head; frame; frame = next) {
        next = frame->next;
        slab_free_frame(alloc, frame);
    }
}

void *kmalloc(size_t size, unsigned int flags)
{
    if (flags != PAL_KERNEL) {
        kprintf(KERN_ERROR, "Unrecognized flags, returning NULL...\n");
        return NULL;
    }

    struct slab_allocator *slab = NULL;
    for (int i = 0; i < ARRAY_SIZE(slabs); ++i)
        if (size <= slabs[i].obj_size)
            return slab_malloc(&slabs[i]);
    
    kprintf(KERN_ERROR, "kmalloc does not support allocations of more than 2048B, yet.\n");
    return NULL;
}

void kfree(void *p)
{
    for (int i = 0; i < ARRAY_SIZE(slabs); ++i) {
        if (slab_has_addr(&slabs[i], p)) {
            slab_free(&slabs[i], p);
            return;
        }
    }
}