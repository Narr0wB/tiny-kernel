
#include <arch/x86/gdt.h>
#include <arch/x86/mm/paging.h>
#include <tiny/mm/vasl.h> 

#define INIT_LARGE_PAGE_COUNT 512

.section .data

# Set up temporary kernel stack
.align 4
stack_bottom:
    .skip 8192
stack_top:
.global stack_top

# Set up the initial page map level 4
.align 0x1000
pgd:
    .fill 512, 8, 0

# We are going to map the first 8M of phys memory to:
#    1) Identity
#    2) VASL_KERNEL_BASE
# We are going to map the first 8M of phys memory to itself, and then map the entire physical memory space at VASL_KERNEL_BASE

.set pgdID,   pgd 
.set pgdHIGH, pgd + PGD_INDEX(VASL_KERNEL_BASE) * 8

.align 0x1000
gpud:
    .fill 512, 8, 0

.align 0x1000
tpud:
    .fill 512, 8, 0

# Create temporary GDT
tmp_gdt:
    GDT_ENTRY_ASM(0, 0, 0, 0)
    GDT_ENTRY_ASM(0, 0xFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ, GDT_FLAGS_64BIT)
    GDT_ENTRY_ASM(0, 0xFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE, GDT_FLAGS_64BIT)

tmp_gdt_descriptor:
    .word (tmp_gdt_descriptor - tmp_gdt - 1)
    .quad (tmp_gdt)



.section .text

.global _start
.extern _kentry

_start:
    # Load new stack
    movq $V2P(stack_top), %rsp
    
    # Load temporary descriptor table 
    lgdt V2P(tmp_gdt_descriptor) 

    # Start loading the addresses of pml4 entries 0 and 511 
    movabsq $V2P(pgdID), %rax 
    movabsq $V2P(pgdHIGH), %rbx 

    # Insert in pgd entries 0 and 511 the address of pud (that they will share)
    movq $V2P(tpud), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rax)
    movq $V2P(gpud), (%rbx)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rbx)

    movq $V2P(tpud), %rax 
    movq $V2P(gpud), %rbx 
    movq $0, %rcx
    movq $INIT_LARGE_PAGE_COUNT, %rdx

.insert_loop:
    movq %rcx, (%rax)
    movq %rcx, (%rbx)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_PAGESIZE), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_PAGESIZE | PAGE_FLAG_GLOBAL), (%rbx)
    addq $PAGE_SIZE_1G, %rcx
    addq $8, %rax
    addq $8, %rbx
    dec %rdx
    jnz .insert_loop

    movq $V2P(pgd), %rax
    movq %rax, %cr3

    movabsq $higher_half, %rax
    jmp *%rax 

higher_half:
    # Move rsp to virtual address
    movabsq $VASL_VIRTUAL_BASE, %rax
    addq %rax, %rsp
    movq $0, %rbp

    call _kentry

    addq $0x16, %rsp

    cli
.loop:
    hlt
    jmp .loop
