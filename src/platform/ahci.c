
#include <tiny/list.h>
#include <tiny/mm/vasl.h>
#include <tiny/platform/pcie.h>

#include <tiny/platform/ahci.h>

extern struct list_head pci_devices;

void init_ahci()
{
    struct pci_device *dev = NULL;
    struct pci_device *ahci = NULL;
    list_foreach_entry(&pci_devices, dev, list) {
        if (read8(dev->ecam_base, PCI_CLASS_CODE_OFFSET) == PCI_CLASS_MASS_STORAGE
            && read8(dev->ecam_base, PCI_SUBCLASS_OFFSET) == PCI_SUBCLASS_SATA
            && read8(dev->ecam_base, PCI_PROG_IF_OFFSET) == PCI_PROGIF_AHCI) {
            ahci = dev;
            break;
        }
    }

    if (!ahci)
        return;

    paddr_t abar = read32(ahci->ecam_base, PCI_BAR5_OFFSET) & ~0xFULL;

    u16 cmd = read16(ahci->ecam_base, PCI_COMMAND_OFFSET);
    cmd |= (PCI_CMD_MEM_SPACE | PCI_CMD_BUS_MASTER);
    write16(ahci->ecam_base, PCI_COMMAND_OFFSET, cmd);
}
