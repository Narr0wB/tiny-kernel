
#ifndef MEMORY_H
#define MEMORY_H

#include <tiny/types.h>
#include <tiny/boot/boot.h>
#include <tiny/compiler.h>
#include <tiny/io.h>
#include <arch/mm/paging.h>

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

void init_memory(struct bootinfo *info);
void init_gdt();

void clean_memory_map(struct efi_memory_map *mem_map);

// MEMORY PAGING 

// typedef struct {
//     uint64_t entries[512];
// } page_table_t;

// typedef enum {
//     PAGE_FLAG_PRESENT   = 1 << 0,
//     PAGE_FLAG_READWRITE = 1 << 1,
//     PAGE_FLAG_USER      = 1 << 2
// } PAGE_TABLE_FLAGS;

// #define SWITCH_PAGE_TREE(tree_addr) \
//     __asm__ volatile (\
//         "movq %0, %%cr3"\
//         :\
//         :   "r" (tree_addr)\
//     )

// #define GET_PAGE_TREE(tree_addr)\
//     __asm__ volatile (\
//         "movq %%cr3, %0"\
//         : "=r" (tree_addr)\
//     )

// void map_virt_to_phys(page_table_t *p, paddr_t phys, vaddr_t virt, uint16_t flags);
// void unmap_virt(page_table_t *p, vaddr_t virt);
// paddr_t get_phys_from_virt(page_table_t *p, vaddr_t virt);

// void identity_map_mmap(page_table_t *p, struct efi_memory_map *mmap, off_t offset);

// // UTILS
// void *mmap_allocate_pages(size_t count, struct efi_memory_map mmap);
// void print_mmap(struct efi_memory_map mmap);

#endif // MEMORY_H
