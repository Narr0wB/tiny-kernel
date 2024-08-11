
#include <memory/memory.h>

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

void init_mm() {
    gdt_load(&GDTdescriptor, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
}
