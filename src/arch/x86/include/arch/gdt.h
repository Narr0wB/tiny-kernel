
#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

#define DPL_KERNEL 0
#define DPL_USER   3

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
#define GDT_BASE_HIGH64(b)              ((((uint64_t)(b)) >> 32) & 0xFFFFFFFFULL)

#define GDT_ENTRY_ASM(base, limit, access, flags) \
    .word GDT_LIMIT_LOW(limit), GDT_BASE_LOW(base) ; \
    .byte GDT_BASE_MIDDLE(base), access, GDT_LIMIT_HIGH_FLAGS(limit, flags), GDT_BASE_HIGH(base)

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE 0x18
#define GDT_USER_DATA 0x20

// TSS entry is 16 bytes, so it takes up two slots
#define GDT_ENTRIES 7

#define GDT_ENTRY_NULL      0
#define GDT_ENTRY_KERNEL_CS 1
#define GDT_ENTRY_KERNEL_DS 2
#define GDT_ENTRY_USER_CS   3
#define GDT_ENTRY_USER_DS   4
#define GDT_ENTRY_CPU_TSS   5

#define GDT_ENTRY_NULL_OFF      0 << 3
#define GDT_ENTRY_KERNEL_CS_OFF 1 << 3
#define GDT_ENTRY_KERNEL_DS_OFF 2 << 3
#define GDT_ENTRY_USER_CS_OFF   3 << 3
#define GDT_ENTRY_USER_DS_OFF   4 << 3
#define GDT_ENTRY_CPU_TSS_OFF   5 << 3

#ifndef __ASSEMBLER__ 

#include <tiny/types.h>
#include <tiny/compiler.h>

#define GDT_ENTRY(base, limit, access, flags) (struct gdt_entry) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_LIMIT_HIGH_FLAGS(limit, flags),         \
    GDT_BASE_HIGH(base),                        \
}

#define GDT_ENTRY_16B(ptr, base, limit, access, flags)      \
do {                                                        \
    struct gdt_entry *const _p = (struct gdt_entry *)(ptr); \
    _p[0] = GDT_ENTRY(base, limit, access, flags);          \ 
    const uint64_t _high = GDT_BASE_HIGH64(base);            \
    memcpy(&_p[1], &_high, sizeof(uint64_t));      \
} while(0)

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags_limit_high;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t size;
    uint64_t addr;
} __attribute__((packed));

struct tss_entry {
    uint32_t resvd1;

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint32_t resvd2;
    uint32_t resvd3;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint32_t resvd4;

    uint32_t iopb;
} __attribute__((packed));

static inline void gdt_load(struct gdt_entry *gdt, uint32_t size) {
    static struct gdt_ptr ptr;
    
    ptr.addr = (uintptr_t) gdt; 
    ptr.size = size;

    __asm__ volatile ("lgdt (%0)" :: "r" (&ptr));
}

#endif // __ASSEMBLER__

#endif // ARCH_X86_GDT_H 
