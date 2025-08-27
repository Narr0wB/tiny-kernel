.global gdt_load
gdt_load:
    # SYSV ABI function prologue
    pushq %rbp
    movq %rsp, %rbp 
    
    cli
    lgdt (%rdi)         # Load the gdt table register with the first argument to the function (gdt_descriptor_t*)
    mov %ds, %dx
    mov %es, %dx
    mov %fs, %dx
    mov %gs, %dx
    mov %ss, %dx

    pushq %rsi          # What we need to put in CS
    pushq retf_cs       # Return address after perfoming the far return
    lretq 

retf_cs:
    # SYSV ABI function epilogue
    movq %rbp, %rsp 
    popq %rbp
    retq


