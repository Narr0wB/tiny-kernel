
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/asm.h>
#include <arch/x86/cpu.h>
#include <arch/x86/backtrace.h>

#include <tiny/mm/bootmem.h>
#include <tiny/mm/types.h>
#include <tiny/mm/vasl.h>
#include <tiny/compiler.h>
#include <tiny/io.h>
#include <tiny/errno.h>

#include <arch/x86/mm/paging.h>


static void page_fault_handler(struct irq_frame *frame, void *data) 
{
    paddr_t cr2 = cpu_get_cr2();
    kprintf(KERN_ERROR, "Had a pagefault trying to access addr %p, error code: %d"EOL, cr2, frame->err_code);
    dump_stack_backtrace((void*)frame->rsp, KERN_ERROR);

    while (1)
        hlt();
}


static struct irq_handler pgft_handler = IRQ_HANDLER_INIT("Page fault handler", page_fault_handler, NULL, 0, pgft_handler);


void init_paging(struct memory_info *info)
{
    /* Enable global paging */
    uint64_t cr4 = cpu_get_cr4();
    cr4 |= CR4_PGE;
    cpu_set_cr4(cr4);

    kprintf(KERN_INFO, "Setting up the main kernel pagetable with global pages enabled...\n");


    /* 
     * Since exception no. 14 is the only CPU exception that we dont handle with the default 
     * unhandled_cpu_exception(), register our own page fault handler
     */
    register_irq_handler(14, &pgft_handler);
}


int vm_map(pgd_t *pgd, paddr_t phys, vaddr_t virt, size_t size, u64 flags, vm_pt_alloc_t allocator)
{
    if (!size)
        return 0;

    if (!pgd || !allocator || ((phys | virt | size) & (PAGE_SIZE - 1))
        || size - 1 > UINT64_MAX - virt
        || phys > (PHYS_ADDR_MASK | (PAGE_SIZE - 1))
        || size - 1 > (PHYS_ADDR_MASK | (PAGE_SIZE - 1)) - phys
        || (flags & ~((u64)PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE
                      | PAGE_FLAG_USER | PAGE_FLAG_GLOBAL | PAGE_FLAG_PAGESIZE | 0x18)))
        return -EINVAL;

    /* Four-level paging: the whole range must stay in one canonical half. */
    u64 half = virt >> 47;
    if ((half != 0 && half != 0x1ffff) || ((virt + size - 1) >> 47) != half)
        return -EINVAL;

    flags = (flags & ~(u64)PAGE_FLAG_PAGESIZE) | PAGE_FLAG_PRESENT;

    /* Pass 1: allocate tables and validate the entire range. */
    paddr_t pa = phys;
    vaddr_t va = virt;
    size_t remaining = size;
    while (remaining) {
        struct page_table *table = pgd;
        for (int shift = 39; shift >= 12; shift -= 9) {
            u64 *entry = &table->entry[(va >> shift) & 511];
            size_t span = 1ULL << shift;
            bool leaf = shift == 12
                || ((shift == 21 || shift == 30)
                    && !((pa | va) & (span - 1)) && remaining >= span && !*entry);
            if (leaf) {
                if (*entry)
                    return -EEXIST;
                pa += span;
                va += span;
                remaining -= span;
                break;
            }

            if (*entry) {
                if (!(*entry & PAGE_FLAG_PRESENT) || (*entry & PAGE_FLAG_PAGESIZE))
                    return -EEXIST;
                u64 permissions = flags & (PAGE_FLAG_READWRITE | PAGE_FLAG_USER);
                if ((*entry & permissions) != permissions || (*entry & (1ULL << 63)))
                    return -EINVAL;
                table = (struct page_table *)p_to_v(*entry & PHYS_ADDR_MASK);
            } else {
                struct vm_pt_page page;
                int err = allocator(&page, NULL);
                if (err)
                    return err;
                table = page.virt;
                for (size_t i = 0; i < 512; ++i)
                    table->entry[i] = 0;
                *entry = page.phys | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_USER;
            }
        }
    }

    /* Pass 2: install leaves; all required tables now exist. */
    pa = phys;
    va = virt;
    remaining = size;
    while (remaining) {
        struct page_table *table = pgd;
        for (int shift = 39; shift >= 12; shift -= 9) {
            u64 *entry = &table->entry[(va >> shift) & 511];
            size_t span = 1ULL << shift;
            bool leaf = shift == 12
                || ((shift == 21 || shift == 30)
                    && !((pa | va) & (span - 1)) && remaining >= span && !*entry);
            if (leaf) {
                *entry = pa | flags | (shift == 12 ? 0 : PAGE_FLAG_PAGESIZE);
                pa += span;
                va += span;
                remaining -= span;
                break;
            }
            table = (struct page_table *)p_to_v(*entry & PHYS_ADDR_MASK);
        }
    }
    return 0;
}
