
#ifndef VASL_H
#define VASL_H

#define VASL_VIRTUAL_BASE           0xFFFFFF8000000000 
#define VASL_KERNEL_BASE            VASL_VIRTUAL_BASE + 0x100000
#define VASL_KERNEL_STACK_BOTTOM    VASL_KERNEL_BASE + 0xB00000
#define VASL_KERNEL_STACK_TOP       VASL_KERNEL_BASE + 0xC00000

#define VASL_KERNEL_PHYS_BASE       VASL_KERNEL_BASE + 0xD00000

#define V2P(x) ((x) - VASL_VIRTUAL_BASE)
#define P2V(x) ((x) + VASL_VIRTUAL_BASE)

#define VASL_USER_EXEC_BASE         0x00007FF000000000
#define VASL_USER_HEAP_BASE         0x00007FF00C000000

#ifndef __ASSEMBLER__

#include <tiny/types.h>

#define v_to_p(x) (paddr_t)V2P((vaddr_t)(x))
#define p_to_v(x) (vaddr_t)P2V((paddr_t)(x))

#endif // __ASSEMBLER__

#endif // VASL_H