
#include <arch/cpu.h>
#include <arch/mm/paging.h>

// CPU struct for CPU0
static struct cpu_info cpu0;

static void cpu_setup_gdt(struct cpu_info *cpu) {
    cpu->gdt[GDT_ENTRY_NULL]      = GDT_ENTRY(0, 0, 0, 0);
    cpu->gdt[GDT_ENTRY_KERNEL_CS] = GDT_ENTRY(0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ, GDT_FLAGS_64BIT);
    cpu->gdt[GDT_ENTRY_KERNEL_DS] = GDT_ENTRY(0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE, GDT_FLAGS_64BIT);
    cpu->gdt[GDT_ENTRY_USER_CS]   = GDT_ENTRY(0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ, GDT_FLAGS_64BIT);
    cpu->gdt[GDT_ENTRY_USER_DS]   = GDT_ENTRY(0, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE, GDT_FLAGS_64BIT);

    // cpu->gdt[GDT_ENTRY_CPU_TSS]   = GDT_ENTRY((uintptr_t) &(cpu->tss), sizeof(struct tss_entry) - 1, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DESCRIPTOR_TSS, 0);
    // cpu->gdt[GDT_ENTRY_CPU_TSS+1] = GDT_ENTRY16_HIGH((uintptr_t) &(cpu->tss));
    GDT_ENTRY_16B(&cpu->gdt[GDT_ENTRY_CPU_TSS], (uintptr_t) &(cpu->tss), sizeof(struct tss_entry) - 1, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DESCRIPTOR_TSS, 0);

    gdt_load(cpu->gdt, sizeof(cpu->gdt) - 1);

    // Reload CS register
    __asm__ volatile (
        "movabsq $1f, %%rax\n"
        "pushq %[kcs]\n"
        "pushq %%rax\n"
        "lretq\n"

        "1:\n"
        "movw %[kds], %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        :: [kcs] "i" (GDT_ENTRY_KERNEL_CS_OFF), [kds] "i" (GDT_ENTRY_KERNEL_DS_OFF)
        : "rax", "memory"
    );
}

void cpu_init_tss(struct cpu_info *cpu) {
    memset(&cpu->tss, 0, sizeof(struct tss_entry));
    
    /* 
     * The iopb field of the TSS entry is the offset into the TSS entry where 
     * the IO Permission Bitmap resides. If such offset is >= than the size of the entry
     * then no bitmap is used.
     */
    cpu->tss.iopb = sizeof(struct tss_entry);
}

void cpu_init_boot() {
    cpu_setup_gdt(&cpu0);
}

void cpu_setup_main_core() {
    cpu_init_tss(&cpu0);
    cpu0.cpu_id = 0;
}