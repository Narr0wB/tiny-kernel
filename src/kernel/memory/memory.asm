global gdt_load
gdt_load:
    ; SYSV ABI function prologue
    push rbp
    mov rbp, rsp
   
    lgdt 6[rdi]       ; Load the gdt table register with the first argument to the function (gdt_descriptor_t*)
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx

    push rsi          ; Put on the stack the index of which GDT entry will be used to describe our code segment
    push .retf_cs     ; Put on the stack the return address after perfoming the far return
    retf 

.retf_cs:
    ; SYSV ABI function epilogue
    mov rsp, rbp
    pop rbp
    ret
