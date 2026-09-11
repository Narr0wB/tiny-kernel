#ifndef ACPI_H
#define ACPI_H

#include <tiny/types.h>
#include <tiny/compiler.h>

struct acpi_rsdp_descriptor {
    char     signature[8];
    u8       checksum;
    char     oem_id[6];
    u8       revision;
    u32      rsdt_address;
    u32      length;
    u64      xsdt_address;
    u8       extended_checksum;
    u8       reserved[3];
} __packed;

struct acpi_sdt_header {
    char     signature[4];
    u32      length;
    u8       revision;
    u8       checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    u32      oem_revision;
    u32      creator_id;
    u32      creator_revision;
} __packed;

struct mcfg_entry {
    u64      base_address;
    u16      pci_segment_group;
    u8       start_pci_bus;
    u8       end_pci_bus;
    u32      reserved;
} __packed;

struct mcfg_table {
    struct acpi_sdt_header header;
    u64                    reserved;
    struct mcfg_entry      entries[];
} __packed;

void init_acpi(paddr_t rsdp);

#endif // ACPI_H
