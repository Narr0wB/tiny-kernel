
#include <arch/idt.h>
#include <arch/irq.h>
#include <arch/asm.h>
#include <arch/cpu.h>
#include <tiny/mm/bootmem.h>
#include <tiny/mm/types.h>
#include <tiny/mm/vasl.h>
#include <tiny/compiler.h>
#include <tiny/io.h>

#include <arch/mm/paging.h>

__align(0x1000) pgd_t kernel_pgd = {0};
__align(0x1000) pud_t kernel_pud = {0};

static void page_fault_handler(struct irq_frame *frame, void *data) 
{
    paddr_t cr2 = cpu_get_cr2();

    kprintf(KERN_ERROR, "Had a pagefault trying to access addr %p, error code: %d", cr2, frame->err_code);

    while (1) {
        hlt();
    }
}


static struct irq_handler pgft_handler = IRQ_HANDLER_INIT("Page fault handler", page_fault_handler, NULL, 0, pgft_handler);

/*
 * We are going to use 2MB pages to map the virtual kernel address space onto the physical address space.
 * We are then going to mark all of this pages as global, so that any process that will be subsequently 
 * created will have in it's address space, protected by prilege, the entire kernel virtual address space 
 * (i.e. Without mapping in the process' page tables the kernel virtual address space).
 * 
 * We are going to start at VASL_VIRTUAL_BASE and stop until VASL_VIRTUAL_BASE + (max_pfn << 12).
 */
void create_kernel_pagetable(pn_t max_pfn)
{
    int pgd_index = PGD_INDEX(VASL_VIRTUAL_BASE);
    kernel_pgd.entry[pgd_index] = (v_to_p(&kernel_pud) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_READWRITE);

    pmd_t *curr_pmd = NULL;
    for (pn_t pfn = 0; pfn < max_pfn; pfn += 512) {
        paddr_t phys_addr = pn_to_paddr(pfn);
        vaddr_t virt_addr = P2V(phys_addr);

        int pud_index = PUD_INDEX(virt_addr);
        if (kernel_pud.entry[pud_index] == 0) {
            curr_pmd = bootmem_alloc(PAGE_SIZE, PAGE_ALIGNMENT);
            kernel_pud.entry[pud_index] = (v_to_p(curr_pmd) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_READWRITE);
        } 

        int pmd_index = PMD_INDEX(virt_addr);
        curr_pmd->entry[pmd_index] = (PAGE_ALIGN(phys_addr) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_PAGESIZE | PAGE_FLAG_READWRITE);
    }
}

void setup_kernel_paging(struct memory_info *info)
{
    /* Enable global paging */
    uint64_t cr4 = cpu_get_cr4();
    cr4 |= CR4_PGE;
    cpu_set_cr4(cr4);

    kprintf(KERN_INFO, "Setting up the main kernel pagetable with global pages enabled...\n");
    create_kernel_pagetable(info->max_pfn);

    /* 
     * Since exception no. 14 is the only CPU exception that we dont handle with the default 
     * unhandled_cpu_exception(), register our own page fault handler
     */
    register_irq_handler(14, &pgft_handler);

    cpu_set_cr3(v_to_p(&kernel_pgd));
}