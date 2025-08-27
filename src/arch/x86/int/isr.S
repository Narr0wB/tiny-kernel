
#include <arch/registers.h>

.extern exception_handler 
.extern irq_handler

isr_stub:
    PUSHAQ 

    movq 120(%rsp), %rdi    # load irq number
    movq 128(%rsp), %rsi    # load err code
    leaq 136(%rsp), %rdx    # load the interrupt_info_t structure
    movq %rsp, %rcx         # load the saved registers
    
    call exception_handler 
    
    POPAQ 
    addq $16, %rsp 
    iretq

irq_stub:
    PUSHAQ 

    movq 120(%rsp), %rdi    # load irq number
    call irq_handler
    
    POPAQ 
    addq $8, %rsp 
    iretq

.macro isr_err_stub num
isr_stub_\num:
    pushq $\num
    jmp isr_stub
.endm

.macro isr_no_err_stub num
isr_stub_\num:
    pushq $0
    pushq $\num
    jmp isr_stub
.endm

.macro irq num
irq_\num:
    pushq $\num
    jmp irq_stub
.endm

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq 0
irq 1
irq 2 
irq 3
irq 4
irq 5
irq 6
irq 7
irq 8
irq 9
irq 10
irq 11 
irq 12 
irq 13 
irq 14 
irq 15 


.section .data

.global isr_stub_table
.align 8
isr_stub_table:
.rept 32
    .quad isr_stub_\+
.endr

.global irq_table
.align 8
irq_table:
.rept 16
    .quad irq_\+
.endr