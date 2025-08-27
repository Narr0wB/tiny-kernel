
#ifndef GDT_ARCH_X86_H
#define GDT_ARCH_X86_H

#ifndef __ASSEMBLER__ 

#include <types.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags_limit_high;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t addr;
} __attribute__((packed)) gdt_descriptor_t;

#endif

#define GDT_ACCESS_PRESENT                   1 << 7

#define GDT_ACCESS_RING0                     0 << 5
#define GDT_ACCESS_RING1                     1 << 5
#define GDT_ACCESS_RING2                     2 << 5 
#define GDT_ACCESS_RING3                     3 << 5

#define GDT_ACCESS_DATA_SEGMENT              1 << 4
#define GDT_ACCESS_CODE_SEGMENT              3 << 3
#define GDT_ACCESS_DESCRIPTOR_TSS            0 << 4

#define GDT_ACCESS_CODE_READ                 1 << 1
#define GDT_ACCESS_DATA_WRITE                1 << 1

#define GDT_ACCESS_CODE_CONFORMING           1 << 2
#define GDT_ACCESS_DATA_DIRECTION_UP         0 << 2
#define GDT_ACCESS_DATA_DIRECTION_DOWN       1 << 2

#define GDT_FLAGS_64BIT                0x20
#define GDT_FLAGS_32BIT                0x40
#define GDT_FLAGS_16BIT                0x00
#define GDT_FLAGS_GRANULARITY_BYTE     0x00
#define GDT_FLAGS_GRANULARITY_PAGE     0x80

#define GDT_LIMIT_LOW(l)                (l & 0xFFFF)
#define GDT_BASE_LOW(b)                 (b & 0xFFFF)
#define GDT_BASE_MIDDLE(b)              ((b >> 16) & 0xFF)
#define GDT_LIMIT_HIGH_FLAGS(l, f)      (((l >> 16) & 0xF) | (f & 0xF0))
#define GDT_BASE_HIGH(b)                ((b >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_LIMIT_HIGH_FLAGS(limit, flags),         \
    GDT_BASE_HIGH(base),                        \
}

#define GDT_ENTRY_ASM(base, limit, access, flags) \
    .word GDT_LIMIT_LOW(limit), GDT_BASE_LOW(base) ; \
    .byte GDT_BASE_MIDDLE(base), access, GDT_LIMIT_HIGH_FLAGS(limit, flags), GDT_BASE_HIGH(base)

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE 0x18
#define GDT_USER_DATA 0x20

#endif // GDT_ARCH_X86_H
