[BITS 64]

global load_idt 
load_idt:
    ; SYSV ABI function prologue
    push rbp
    mov rbp, rsp

    lidt [rdi]
    sti
    
    ; SYSV ABI function epilogue
    mov rsp, rbp
    pop rbp
    ret
