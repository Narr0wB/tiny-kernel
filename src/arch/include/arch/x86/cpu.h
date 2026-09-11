
#ifndef ARCH_X86_CPU_H
#define ARCH_X86_CPU_H

#include <arch/x86/gdt.h>
#include <tiny/compiler.h>

#define CR4_PGE 1 << 7

struct task;

struct cpu_info {
    int cpu_id;

    struct tss_entry tss;

    struct task *current;
    struct gdt_entry gdt[GDT_ENTRIES];
};

void cpu_init_boot();
void cpu_setup_main_core();

static __force_inline uint64_t cpu_get_cr4()
{
    uint64_t cr4;
    __asm__ volatile ("movq %%cr4, %0" : "=r" (cr4));
    return cr4;
}

static __force_inline void cpu_set_cr4(uint64_t cr4)
{
    __asm__ volatile ("movq %0, %%cr4" :: "r" (cr4));
}

static __force_inline uint64_t cpu_get_cr3()
{
    uint64_t cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r" (cr3));
    return cr3;
}

static __force_inline void cpu_set_cr3(uint64_t cr3)
{
    __asm__ volatile ("movq %0, %%cr3" :: "r" (cr3));
}

static __force_inline uint64_t cpu_get_cr2()
{
    uint64_t cr2;
    __asm__ volatile ("movq %%cr2, %0" : "=r" (cr2));
    return cr2;
}

#endif // ARCH_X86_CPU_H
