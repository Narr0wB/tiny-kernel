#/usr/bin/python3

with open("pt_mappings.S", "w") as f:
    print(f"#include <arch/mm/paging.h>", file=f)
    print(f"", file=f)
    print(f".set phys, 0", file=f)
    for i in range(4):
        print(f".global pd{i}", file=f)
        print(f".align 0x1000", file=f)
        print(f"pd{i}:", file=f)
        print(f".rept 512", file=f)
        print(f"    .quad ((phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)", file=f)
        print(f"    .set phys, phys + 1", file=f)
        print(f".endr", file=f)