%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15 
%endmacro

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

isr_stub:
    pushaq

    mov rdi, [rsp + 120] ; load irq number
    mov rsi, [rsp + 128] ; load err code
    lea rdx, [rsp + 136] ; load the interrupt_info_t structure
    mov rcx, rsp         ; load the saved registers
    
    call exception_handler 
    
    popaq
    add rsp, 16
    iretq

irq_stub:
    pushaq

    mov rdi, [rsp + 120] ; load irq number
    call irq_handler
    
    popaq
    add rsp, 8 
    iretq

%macro isr_err_stub 1
isr_stub_%+%1:
    push %1
    jmp isr_stub 
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0
    push %1
    jmp isr_stub
%endmacro

%macro irq 1
irq_%+%1:
    push %1
    jmp irq_stub
%endmacro
    
extern exception_handler 
extern irq_handler

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


global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i
%assign i i+1 
%endrep

global irq_table
irq_table:
%assign i 0 
%rep    16 
    dq irq_%+i
%assign i i+1 
%endrep
