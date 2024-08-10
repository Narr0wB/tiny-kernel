
#include <memory/memory.h>

gdt_entry_t GDT[] = {
    GDT_ENTRY(0, 0, 0, 0),
    // Kernel code segment entry
    GDT_ENTRY(
        0,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READ,
        GDT_FLAGS_64BIT | GDT_FLAGS_GRANULARITY_BYTE
    ),
    // Kernel data segment entry
    GDT_ENTRY(
        0,
        0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITE,
        GDT_FLAGS_64BIT | GDT_FLAGS_GRANULARITY_BYTE
    ),
};

gdt_descriptor_t GDTdescriptor = { (sizeof(GDT) - 1), GDT };

extern void gdt_load(gdt_descriptor_t *gdt_descriptor, uint32_t code_segment, uint32_t data_segment);

void init_mm() {
    gdt_load(&GDTdescriptor, 1 * sizeof(gdt_entry_t), 2 * sizeof(gdt_entry_t));
}
