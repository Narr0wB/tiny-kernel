
#include <arch/x86/registers.h>

.extern __panic
.global panic
panic:
    pushq %rbp
    movq %rsp, %rbp

    PUSHAQ 
    movq %rsp, %rsi
    call __panic

loop:
    hlt
    jmp loop
