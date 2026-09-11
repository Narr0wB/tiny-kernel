#ifndef PCIE_H
#define PCIE_H

#include <tiny/types.h>
#include <tiny/compiler.h>
#include <tiny/platform/acpi.h>
#include <tiny/list.h>
#include <tiny/mm/vmm.h>

/* PCI configuration-space offsets */
#define PCI_VENDOR_ID_OFFSET         0x00
#define PCI_DEVICE_ID_OFFSET         0x02
#define PCI_COMMAND_OFFSET           0x04
#define PCI_STATUS_OFFSET            0x06
#define PCI_REVISION_ID_OFFSET       0x08
#define PCI_PROG_IF_OFFSET           0x09
#define PCI_SUBCLASS_OFFSET          0x0A
#define PCI_CLASS_CODE_OFFSET        0x0B
#define PCI_CACHE_LINE_SIZE_OFFSET   0x0C
#define PCI_LATENCY_TIMER_OFFSET     0x0D
#define PCI_HEADER_TYPE_OFFSET       0x0E
#define PCI_BIST_OFFSET              0x0F
#define PCI_BAR0_OFFSET              0x10
#define PCI_BAR1_OFFSET              0x14
#define PCI_BAR2_OFFSET              0x18
#define PCI_BAR3_OFFSET              0x1C
#define PCI_BAR4_OFFSET              0x20
#define PCI_BAR5_OFFSET              0x24
#define PCI_CARDBUS_CIS_PTR_OFFSET   0x28
#define PCI_SUBSYSTEM_VENDOR_ID_OFFSET 0x2C
#define PCI_SUBSYSTEM_ID_OFFSET      0x2E
#define PCI_EXPANSION_ROM_BASE_OFFSET 0x30
#define PCI_CAPABILITIES_PTR_OFFSET  0x34
#define PCI_RESERVED_OFFSET          0x35
#define PCI_INTERRUPT_LINE_OFFSET    0x3C
#define PCI_INTERRUPT_PIN_OFFSET     0x3D
#define PCI_MIN_GRANT_OFFSET         0x3E
#define PCI_MAX_LATENCY_OFFSET       0x3F

#define PCI_VENDOR_NO_DEVICE        0xFFFF

#define PCI_CLASS_MASS_STORAGE      0x01
#define PCI_SUBCLASS_SATA           0x06
#define PCI_PROGIF_AHCI             0x01

#define PCI_CMD_IO_SPACE            (1U << 0)
#define PCI_CMD_MEM_SPACE           (1U << 1)
#define PCI_CMD_BUS_MASTER          (1U << 2)

#define PCI_MAX_DEVICES             32
#define PCI_MAX_FUNCTIONS           8
#define PCI_MAX_BARS                8

#define PCI_HEADER_TYPE_MULTIFUNC   0x80
#define PCI_HEADER_TYPE_MASK        0x7F

struct pci_device {
    u8               bus;
    u8               device;
    u8               function;
    vaddr_t          ecam_base;
    struct vm_area   *bars[PCI_MAX_BARS];
    struct list_head list;
};

void init_pci();


#endif // PCIE_H
