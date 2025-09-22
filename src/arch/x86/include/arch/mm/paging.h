
#ifndef ARCH_X86_PAGING_H
#define ARCH_X86_PAGING_H

#define ALIGN_UP(x, a) ((typeof(x))(((unsigned long)(x) + (a) - 1) & ~(a - 1)))
#define ALIGN_DOWN(x, a) ((typeof(x))(((unsigned long)(x)) & ~(a - 1)))

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PAGE_ALIGNMENT PAGE_SIZE
#define PAGE_MASK (~(PAGE_SIZE-1))
#define SIZE_TO_PAGES(size) (((size_t)(size) + PAGE_SIZE - 1)/PAGE_SIZE)

#define PAGE_ALIGN_UP(addr)         ALIGN_UP(addr, PAGE_SIZE) 
#define PAGE_ALIGN_DOWN(addr)       ALIGN_DOWN(addr, PAGE_SIZE) 
#define PAGE_ALIGN(addr)            PAGE_ALIGN_UP(addr)
#define IS_PAGE_ALIGNED(addr)       (! (addr & ~PAGE_MASK))
#define PHYS_ADDR_MASK              0xFFFFFFFFFFFFF000

#define PAGE_FLAG_PRESENT   1 << 0
#define PAGE_FLAG_READWRITE 1 << 1
#define PAGE_FLAG_USER      1 << 2
#define PAGE_FLAG_PAGESIZE  1 << 7
#define PAGE_FLAG_GLOBAL    1 << 8

#define PGD_INDEX(x)  (((x) >> 39) & 511)
#define PUD_INDEX(x)  (((x) >> 30) & 511)
#define PMD_INDEX(x)  (((x) >> 21) & 511)
#define PD_INDEX(x)   (((x) >> 12) & 511)

#define pn_to_paddr(pn)   (pn << PAGE_SHIFT)
#define paddr_to_pn(addr) (addr >> PAGE_SHIFT)

#ifndef __ASSEMBLER__

#include <tiny/types.h>
#include <tiny/mm/types.h>

struct page_table {
    uint64_t entry[512];
};

typedef struct page_table pgd_t; /* Page global directory */
typedef struct page_table pud_t; /* Page upper directory */
typedef struct page_table pmd_t; /* Page middle directory */
typedef struct page_table pd_t;  /* Page directory*/

void setup_kernel_paging(struct memory_info *info);


#endif // __ASSEMBLER__

#endif // ARCH_X86_PAGING_H