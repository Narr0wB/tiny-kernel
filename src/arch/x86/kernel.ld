
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

VASL_VIRTUAL_BASE = 0xFFFFFF8000000000;
VASL_KERNEL_BASE = VASL_VIRTUAL_BASE + 0x100000;

SECTIONS {
    . = VASL_KERNEL_BASE;
    
    __pkernel_start = . - VASL_VIRTUAL_BASE;
    .text ALIGN(0x1000) : AT(ADDR(.text) - VASL_VIRTUAL_BASE)
    {
        *(.text)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - VASL_VIRTUAL_BASE)
    {
        *(.data)
    } 

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - VASL_VIRTUAL_BASE)
    {
        *(.rodata)
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - VASL_VIRTUAL_BASE)
    {
        *(COMMON)
        *(.bss)
    }
    __pkernel_end = . - VASL_VIRTUAL_BASE;
}