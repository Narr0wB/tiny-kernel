
#ifndef MEMORY_H
#define MEMORY_H

#include <common.h>
#include <boot/boot.h>
#include <util/string.h>
#include <util/io.h>

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


#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))
#define PAGE_SIZE 4096 // 4KB page size
#define SIZE_TO_PAGES(size) (((size_t)size + PAGE_SIZE - 1)/PAGE_SIZE)

#define PAGE_ALIGN_DOWN(addr)       (addr & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)         (addr + (PAGE_SIZE - addr % PAGE_SIZE))
#define PHYS_ADDR_MASK              0xFFFFFFFFFFFFF000

typedef struct {
    paddr_t kernel_start;
    paddr_t kernel_end;
} memory_info_t;

typedef enum {
    EFI_RESERVED_MEMORY_TYPE,
    EFI_LOADER_CODE,
    EFI_LOADER_DATA,
    EFI_BOOT_SERVICES_CODE,
    EFI_BOOT_SERVICES_DATA,
    EFI_RUNTIME_SERVICES_CODE,
    EFI_RUNTIME_SERVICES_DATA,
    EFI_CONVENTIONAL_MEMORY,
    EFI_UNUSABLE_MEMORY,
    EFI_ACPI_RECLAIM_MEMORY,
    EFI_ACPI_MEMORY_NVS,
    EFI_MEMORY_MAPPED_IO,
    EFI_MEMORY_MAPPED_IO_PS,
    EFI_PAL_CODE
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

void init_memory(memory_map_t mem_map, paddr_t kernel_start, paddr_t kernel_end);
void init_gdt();

void clean_memory_map(memory_map_t *mem_map);

// MEMORY PAGING 

typedef struct {
    uint64_t entries[512];
} page_table_t;

typedef enum {
    PAGE_FLAG_PRESENT   = 1 << 0,
    PAGE_FLAG_READWRITE = 1 << 1,
    PAGE_FLAG_USER      = 1 << 2
} PAGE_TABLE_FLAGS;

#define SWITCH_PAGE_TREE(tree_addr) \
    __asm__ volatile (\
        "movq %0, %%cr3"\
        :\
        :   "r" (tree_addr)\
    )

#define GET_PAGE_TREE(tree_addr)\
    __asm__ volatile (\
        "movq %%cr3, %0"\
        : "=r" (tree_addr)\
    )

void map_phys_to_virt(page_table_t *p, paddr_t phys, vaddr_t virt, uint16_t flags);
void unmap_virt(page_table_t *p, vaddr_t virt);

paddr_t get_phys_from_virt(page_table_t *p, vaddr_t virt);

void identity_map_mmap(page_table_t *p, memory_map_t *mmap);

// MEMORY ALLOCATION
void *mmap_allocate_pages(size_t pages);

#endif // MEMORY_H
