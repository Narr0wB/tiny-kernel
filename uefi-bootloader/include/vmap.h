
#ifndef VMAP_H
#define VMAP_H

#include <stdint.h>
#include <utils.h>

typedef struct {
    uint64_t entries[512];
} page_table_t;

typedef enum {
    PAGE_FLAG_PRESENT   = 1 << 0,
    PAGE_FLAG_READWRITE = 1 << 1,
    PAGE_FLAG_USER      = 1 << 2
} PAGE_TABLE_FLAGS;

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))
#define PAGE_SIZE 4096 // 4KB page size
#define SIZE_TO_PAGES(size) (((size_t)size + PAGE_SIZE - 1)/PAGE_SIZE)

#define PAGE_ALIGN_DOWN(addr)       (addr & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)         (addr + (PAGE_SIZE - addr % PAGE_SIZE))
#define IS_PAGE_ALIGNED(addr)       (! (addr & ~PAGE_MASK))
#define PHYS_ADDR_MASK              0xFFFFFFFFFFFFF000

#define SWITCH_PAGE_TREE(tree_addr) \
    __asm__ volatile (\
        "movq %0, %%cr3"\
        :\
        :   "r" (tree_addr)\
    )

#define GET_PAGE_TREE(tree_addr)\
    __asm__ volatile (\
        "movq %%cr3, %0"\
        : "=r" (tree_addr)\
    )

void map_virt_to_phys(page_table_t *p, paddr_t phys, vaddr_t virt, uint16_t flags);
void unmap_virt(page_table_t *p, vaddr_t virt);
paddr_t get_phys_from_virt(page_table_t *p, vaddr_t virt);

void identity_map_mmap(page_table_t *p, memory_map_t *mmap);

#endif // VMAP_H