
#ifndef PAGING_ARCH_X86_H
#define PAGING_ARCH_X86_H

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))
#define SIZE_TO_PAGES(size) (((size_t)size + PAGE_SIZE - 1)/PAGE_SIZE)

#define PAGE_ALIGN_DOWN(addr)       (addr & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)         (addr + (PAGE_SIZE - addr % PAGE_SIZE))
#define IS_PAGE_ALIGNED(addr)       (! (addr & ~PAGE_MASK))
#define PHYS_ADDR_MASK              0xFFFFFFFFFFFFF000

#define PAGE_FLAG_PRESENT   1 << 0
#define PAGE_FLAG_READWRITE 1 << 1
#define PAGE_FLAG_USER      1 << 2

#endif // PAGING_ARCH_X86_H