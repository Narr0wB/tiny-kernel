
#ifndef MEMORY_H
#define MEMORY_H

#include <common.h>
#include <boot/boot.h>
#include <util/string.h>
#include <tty/tty.h>

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

typedef enum {
    GDT_ACCESS_CODE_READ                = 0x02,
    GDT_ACCESS_DATA_WRITE               = 0x02,
    
    GDT_ACCESS_CODE_CONFORMING          = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL    = 0x00,
    GDT_ACCESS_DATA_DIRECTION_REVERSE   = 0x04,

    GDT_ACCESS_DATA_SEGMENT             = 0x10,
    GDT_ACCESS_CODE_SEGMENT             = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS           = 0x00,

    GDT_ACCESS_RING0                    = 0x00,
    GDT_ACCESS_RING1                    = 0x20,
    GDT_ACCESS_RING2                    = 0x40, 
    GDT_ACCESS_RING3                    = 0x60,

    GDT_ACCESS_PRESENT                  = 0x80
} GDT_ACCESS;

typedef enum {
    GDT_FLAGS_64BIT               = 0x20,
    GDT_FLAGS_32BIT               = 0x40,
    GDT_FLAGS_16BIT               = 0x00,

    GDT_FLAGS_GRANULARITY_BYTE    = 0x00,
    GDT_FLAGS_GRANULARITY_PAGE    = 0x80,
} GDT_FLAGS;

#define GDT_LIMIT_LOW(l)                (l & 0xFFFF)
#define GDT_BASE_LOW(b)                 (b & 0xFFFF)
#define GDT_BASE_MIDDLE(b)              ((b >> 16) & 0xFF)
#define GDT_LIMIT_HIGH_FLAGS(l, f)      (((l >> 16) & 0xF) | (f & 0xF0))
#define GDT_BASE_HIGH(b)                 ((b >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_LIMIT_HIGH_FLAGS(limit, flags),         \
    GDT_BASE_HIGH(base),                        \
}

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE 0x18
#define GDT_USER_DATA 0x20

#define PAGE_SIZE 4096 // 4KB page size

typedef enum {
    EFI_CONVENTIONAL_MEMORY = 7
} EFI_MEMORY_TYPE;

static const char *EFI_MEMORY_TYPE_STRING[] = {
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory",
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "EfiMemoryMappedIO",
    "EfiMemoryMappedIOPortSpace",
    "EfiPalCode",
};

void init_memory(memory_map_t mem_map);
void init_gdt();

// MEMORY PAGING 

typedef struct {
    uint64_t entries[512];
} page_table_t;

typedef enum {
    PAGE_FLAG_PRESENT   = 1 << 0,
    PAGE_FLAG_READWRITE = 1 << 1,
    PAGE_FLAG_USER      = 1 << 2
} PAGE_TABLE_FLAGS;

#define PHYS_ADDR_MASK   0xFFFFFFFFFFFFF000
#define PAGE_ALIGN       0xFFFFFFFFFFFFF000

void map_virt_to_phys(vaddr_t virt, paddr_t phys, uint16_t flags);
void unmap_virt_to_phys(vaddr_t virt);

paddr_t get_phys_from_virt(vaddr_t virt);

void identity_map_mmap(memory_map_t mmap);

// MEMORY ALLOCATION
void *mmap_allocate_pages(size_t pages);

#endif // MEMORY_H
