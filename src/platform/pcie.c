
#include <tiny/mm/vasl.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/string.h>
#include <tiny/platform/acpi.h>

#include <tiny/platform/pcie.h>

extern struct acpi_sdt_header *acpi_xsdt_header;
extern paddr_t                *acpi_tables;

LIST_HEAD(pci_devices);

static volatile paddr_t pci_ecam_reg(paddr_t base, u8 bus, u8 dev, u8 func)
{
    paddr_t res = base
        + (bus  & 0xFF) << 20
        + (dev  & 0x1F) << 15
        + (func & 0x7)  << 8;
    return (volatile paddr_t)res;
}

void init_pci()
{
    struct mcfg_table *mcfg = NULL;
    size_t xsdt_count = (acpi_xsdt_header->length - sizeof(struct acpi_sdt_header)) / sizeof(paddr_t);
    for (u32 i = 0; i < xsdt_count; ++i) {
        struct acpi_sdt_header *entry = (struct acpi_sdt_header *)p_to_v(acpi_tables[i]);
        if (memcmp(entry->signature, "MCFG", 4) == 0)
            mcfg = (struct mcfg_table *)entry;
    }

    if (!mcfg)
        return;
    
    size_t mcfg_count = (mcfg->header.length - sizeof(struct acpi_sdt_header) - 8) / sizeof(paddr_t);
    for (u32 i = 0; i < mcfg_count; ++i) {
        struct mcfg_entry *entry = &mcfg->entries[i];

        for (u32 bus = entry->start_pci_bus; bus < entry->end_pci_bus; ++bus) {
            for (u32 dev = 0; dev < PCI_MAX_DEVICES; ++dev) {
                for (u32 func = 0; func < PCI_MAX_FUNCTIONS; ++func) {
                    uintptr_t ecam_register = p_to_v(pci_ecam_reg(entry->base_address, bus, dev, func));
                    u16 vendor_id = read16(ecam_register, PCI_VENDOR_ID_OFFSET);
                    if (vendor_id == PCI_VENDOR_NO_DEVICE)
                        continue;

                    struct pci_device *device = kmalloc(sizeof(*device), PAL_KERNEL);
                    if (!device)
                        continue;

                    device->bus = bus;
                    device->device = dev;
                    device->function = func;
                    device->ecam_base = ecam_register;
                    list_add(&device->list, &pci_devices);
                }
            }
        }
    }
}
