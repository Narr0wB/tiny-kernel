
#include <memory/memory.h>

memory_map_t _mmap;

gdt_entry_t GDT[] = {
    GDT_ENTRY(0ULL, 0, 0, 0),

    // Kernel code segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
        GDT_FLAGS_64BIT 
    ),

    // Kernel data segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
        GDT_FLAGS_64BIT 
    ),

    // Userspace code segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
        GDT_FLAGS_64BIT 
    ),

    // Userspace data segment entry
    GDT_ENTRY(
        0ULL,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
        GDT_FLAGS_64BIT 
    ),
};

gdt_descriptor_t GDTdescriptor = {  (sizeof(GDT) - 1), (uintptr_t)GDT  };

extern void gdt_load(gdt_descriptor_t *gdt_descriptor, uint32_t code_segment, uint32_t data_segment);

void init_memory(memory_map_t mem_map) {
    init_gdt();
    _mmap = mem_map;
}

void init_gdt() {
    gdt_load(&GDTdescriptor, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
}

void *mmap_allocate_pages(size_t count) {
    static size_t current_mem_descriptor = 0;
    static paddr_t free_pages            = 0;
    static size_t remaining_pages        = 0;

    // If we dont have enough pages to fulfill the request we need to change the current descriptor and find one big enough for our needs 
    if (remaining_pages < count) {
        for (size_t i = current_mem_descriptor + 1; i < _mmap.size; ++i) {
            memory_descriptor_t *descriptor = (_mmap.map + i);

            if (descriptor->type == EFI_CONVENTIONAL_MEMORY && descriptor->npages >= count) {
                current_mem_descriptor = i; 
                free_pages = descriptor->phys_start;
                remaining_pages = descriptor->npages;

                break;
            }
        }
    }

    void *pages = (void*)(free_pages);
    remaining_pages -= count;
    free_pages += count * PAGE_SIZE;

    return pages; 
}
