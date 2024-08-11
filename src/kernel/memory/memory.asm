global gdt_load
gdt_load:
    ; SYSV ABI function prologue
    push rbp
    mov rbp, rsp
    
    cli
    lgdt [rdi]       ; Load the gdt table register with the first argument to the function (gdt_descriptor_t*)
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    push rsi          ; What we need to put in CS  
    push .retf_cs     ; Return address after perfoming the far return
    retfq 

.retf_cs:
    ; SYSV ABI function epilogue
    mov rsp, rbp
    pop rbp
    ret
