
#include <arch/paging.h>

.set phys, 0
.global pt0
.align 0x1000
pt0:
.rept 512
    .quad ((phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)
    .set phys, phys + 1
.endr

.global pt1
.align 0x1000
pt1:
.rept 512
    .quad ((phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)
    .set phys, phys + 1
.endr

.global pt2
.align 0x1000
pt2:
.rept 512
    .quad ((phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)
    .set phys, phys + 1
.endr

.global pt3
.align 0x1000
pt3:
.rept 512
    .quad ((phys << 12) | PAGE_FLAG_PRESENT | PAGE_FLAG_READWRITE)
    .set phys, phys + 1
.endr