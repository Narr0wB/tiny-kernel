
#include <arch/gdt.h>
#include <arch/mm/paging.h>
#include <tiny/mm/vasl.h> 

.section .data

# Set up temporary kernel stack
.align 4
.global stack_top
stack_bottom:
    .skip 8192
stack_top:

# Set up the initial page map level 4
.align 0x1000
initial_pgd:
    .fill 512, 8, 0

# We are going to identity map the first 8M of phys memory and also map it at an offset (VASL_KERNEL_BASE) 
.set pgdID,   initial_pgd
.set pgdHIGH, initial_pgd + PGD_INDEX(VASL_KERNEL_BASE) * 8

.align 0x1000
pud:
    .fill 512, 8, 0

.align 0x1000
pmd:
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

.global start
.global _start
.extern _kentry

start = V2P(_start)

_start:
    # Load new stack
    movq $V2P(stack_top), %rsp
    
    # Load temporary descriptor table 
    lgdt V2P(tmp_gdt_descriptor) 

    # Start loading the addresses of pml4 entries 0 and 511 
    movabsq $V2P(pgdID), %rax 
    movabsq $V2P(pgdHIGH), %rbx 

    # Insert in pml4 entries 0 and 511 the address of PDPT (that they will share)
    movq $V2P(pud), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rax)
    movq $V2P(pud), (%rbx)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rbx)

    # Insert in PDPT entry n the address of PDT (that pml4 entries 0 and 511 will share again)
    movq $V2P(pud), %rax 
    movq $V2P(pmd), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rax)

    # Insert in PDT the addresses of PT0, PT1, PT2, PT3
    movq $V2P(pmd), %rax
    movq $0, %rbx
    movq $4, %rcx

.pd_insert_loop:
    movq %rbx, (%rax) 
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE | PAGE_FLAG_PAGESIZE), (%rax) # The flag PAGE_FLAG_PAGESIZE allows us to use PMD entries as huge (2MiB) page entries
    addq $0x200000, %rbx
    addq $8, %rax
    dec %rcx
    jnz .pd_insert_loop

    movq $V2P(initial_pgd), %rax
    movq %rax, %cr3

    movabsq $higher_half, %rax
    jmp *%rax 

higher_half:
    # Move rsp to virtual address
    movabsq $VASL_VIRTUAL_BASE, %rax
    addq %rax, %rsp
    addq %rax, %rdi
    movq $0, %rbp

    call _kentry

    addq $0x16, %rsp

    cli
.loop:
    hlt
    jmp .loop
