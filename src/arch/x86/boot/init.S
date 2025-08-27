
#include <arch/gdt.h>
#include <arch/paging.h>
#include <memory/vasl.h> 

.section .data

# Set up temporary kernel stack
.align 4
.global stack_top
stack_bottom:
    .skip 8192
stack_top:

# Set up the initial page map level 4
.align 0x1000
initial_page_map_l4:
    .fill 512, 8, 0

# We are going to identity map the first 8M of phys memory and also map it at an offset (VASL_KERNEL_BASE) 
.set pml4ID,   initial_page_map_l4
.set pml4HIGH, initial_page_map_l4 + 511 * 8

.align 0x1000
pdpt:
    .fill 512, 8, 0

.align 0x1000
pdt:
    .fill 512, 8, 0

.extern pt0
.extern pt1
.extern pt2
.extern pt3

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
.extern _kernel_entry

start = V2P(_start)

_start:
    # Load new stack top
    movq $V2P(stack_top), %rsp
    
    # Load temporary descriptor table 
    lgdt V2P(tmp_gdt_descriptor) 

    # Start loading the addresses of pml4 entries 0 and 511 
    movq $V2P(pml4ID), %rax 
    movq $V2P(pml4HIGH), %rbx 

    # Insert in pml4 entries 0 and 511 the address of PDPT (that they will share)
    movq $V2P(pdpt), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rax)
    movq $V2P(pdpt), (%rbx)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rbx)

    # Insert in PDPT entry 0 the address of PDT (that pml4 entries 0 and 511 will share again)
    movq $V2P(pdpt), %rax 
    movq $V2P(pdt), (%rax)
    orq $(PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE), (%rax)

    # Insert in PDT the addresses of PT0, PT1, PT2, PT3
    movq $V2P(pdt), %rax
    movq $V2P(pt0), %rbx
    movq $4, %rcx

.pt_insert_loop:
    movq %rbx, (%rax) 
    addq 0x1000, %rbx
    addq 8, %rax
    loop .pt_insert_loop

    movq $V2P(initial_page_map_l4), %rax
    movq %rax, %cr3

    jmp higher_half

higher_half:
    # Move rsp to virtual address
    movq $VASL_VIRTUAL_BASE, %rax
    addq %rax, %rsp
    movq $0, %rbp

    call _kernel_entry

    addq $0x16, %rsp

    cli
.loop:
    hlt
    jmp .loop
