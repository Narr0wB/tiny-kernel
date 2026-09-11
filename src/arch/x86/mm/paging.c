
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

#include <arch/x86/mm/paging.h>

__align(0x1000) pgd_t kpgd = {0};
__align(0x1000) pud_t kpud = {0};

static void page_fault_handler(struct irq_frame *frame, void *data) 
{
    paddr_t cr2 = cpu_get_cr2();
    kprintf(KERN_ERROR, "Had a pagefault trying to access addr %p, error code: %d"EOL, cr2, frame->err_code);
    dump_stack_backtrace((void*)frame->rsp, KERN_ERROR);

    while (1)
        hlt();
}
static struct irq_handler pgft_handler = IRQ_HANDLER_INIT("Page fault handler", page_fault_handler, NULL, 0, pgft_handler);

void create_kernel_pagetable(struct memory_info *info)
{
    int pgd_index = PGD_INDEX(VASL_VIRTUAL_BASE);
    kpgd.entry[pgd_index] = (v_to_p(&kpud) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_READWRITE);

    pmd_t *curr_pmd = NULL;
    for (pn_t pfn = 0; pfn < ALIGN_UP(info->max_pfn, 512); pfn += 512) {
        paddr_t phys_addr = pn_to_paddr(pfn);
        vaddr_t virt_addr = P2V(phys_addr);

        int pud_index = PUD_INDEX(virt_addr);
        if (kpud.entry[pud_index] == 0) {
            curr_pmd = bootmem_alloc(PAGE_SIZE, PAGE_ALIGNMENT);
            kpud.entry[pud_index] = (v_to_p(curr_pmd) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_READWRITE);
        } 

        int pmd_index = PMD_INDEX(virt_addr);
        curr_pmd->entry[pmd_index] = (PAGE_ALIGN(phys_addr) | PAGE_FLAG_PRESENT | PAGE_FLAG_GLOBAL | PAGE_FLAG_PAGESIZE | PAGE_FLAG_READWRITE);
    }
}

void init_paging(struct memory_info *info)
{
    /* Enable global paging */
    uint64_t cr4 = cpu_get_cr4();
    cr4 |= CR4_PGE;
    cpu_set_cr4(cr4);

    kprintf(KERN_INFO, "Setting up the main kernel pagetable with global pages enabled...\n");
    create_kernel_pagetable(info);

    /* 
     * Since exception no. 14 is the only CPU exception that we dont handle with the default 
     * unhandled_cpu_exception(), register our own page fault handler
     */
    register_irq_handler(14, &pgft_handler);

    cpu_set_cr3(v_to_p(&kpgd));
}



void vm_map(pgd_t *pgd, paddr_t phys, vaddr_t virt, size_t size, u64 flags, vm_pt_alloc_t allocator)
{
    u32 huge_pages = size / PAGE_SIZE_1G;
    u32 middle_pages = (size - huge_pages * PAGE_SIZE_1G) / PAGE_SIZE_2M;
    u32 pages = (middle_pages - huge_pages * PAGE_SIZE_1G - middle_pages * PAGE_SIZE_2M) / PAGE_SIZE_4K;

    for (size_t page = 0; page < huge_pages; ++page) {
        vaddr_t vaddr = virt + page * PAGE_SIZE_1G;
        paddr_t paddr = phys + page * PAGE_SIZE_1G;

        u32 pgd_index = PGD_INDEX(vaddr);
        pud_t *pud = pgd->entry[pgd_index] & PAGE_MASK;

        if (!pud) {
            struct vm_pt_page pg;
            allocator(&pg, NULL);
            pgd->entry[pgd_index] = pud = pg.phys & PAGE_MASK;
        }

        u32 pud_index = PUD_INDEX(vaddr);
        pud->entry[pud_index] = (paddr & PAGE_MASK) | (flags & ~PAGE_MASK);
    }

    for (size_t page = 0; page < middle_pages; ++page) {
        vaddr_t vaddr = virt + page * PAGE_SIZE_2M;

        u32 pgd_index = PGD_INDEX(vaddr);
        pud_t *pud = pgd->entry[pgd_index] & PAGE_MASK;

        if (!pud) {
            struct vm_pt_page pg;
            allocator(&pg, NULL);
            pgd->entry[pgd_index] = pud = pg.phys & PAGE_MASK;
        }

        u32 pud_index = PUD_INDEX(vaddr);
        u32 pmd_index = PMD_INDEX(vaddr);
    }

    for (size_t page = 0; page < pages; ++page) {
        vaddr_t vaddr = virt + page * PAGE_SIZE_4K;

        u32 pgd_index = PGD_INDEX(vaddr);
        u32 pud_index = PUD_INDEX(vaddr);
        u32 pmd_index = PMD_INDEX(vaddr);
        u32 pd_index  = PD_INDEX(vaddr);
    }
}