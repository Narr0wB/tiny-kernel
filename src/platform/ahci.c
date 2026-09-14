
#include <tiny/list.h>
#include <tiny/errno.h>
#include <tiny/io.h>
#include <tiny/mm/vasl.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/mm/vmm.h>
#include <tiny/platform/pcie.h>
#include <tiny/block/block.h>
#include <arch/x86/timer.h>

#include <tiny/platform/ahci.h>

extern struct list_head pci_devices;
extern struct vm_space kernel_space;


struct hba_mem *controller;
LIST_HEAD(sata_list);
static u32 sata_device_count = 0;

static __force_inline int ahci_wait_port(struct hba_port *port)
{
    u64 deadline = timer_get_ms() + AHCI_PORT_TIMEOUT_MS;
    while (get_bit(port->tfd, 7) || get_bit(port->tfd, 3)) {
        if (timer_get_ms() >= deadline)
            return -ETIMEDOUT;
    }
    return 0;
}


static __force_inline int ahci_port_stop(struct hba_port *port)
{
    port->cmd &= ~((1U << 0) | (1U << 4));
    u64 deadline = timer_get_ms() + AHCI_PORT_STOP_TIMEOUT_MS;
    while (port->cmd & ((1U << 15) | (1U << 14))) {
        if (timer_get_ms() >= deadline)
            return -ETIMEDOUT;
    }
    return 0;
}


static __force_inline int ahci_port_start(struct hba_port *port)
{
    port->serr = port->serr;
    port->is   = port->is;

    set_bit(port->cmd, 4);
    int err = ahci_wait_port(port);
    if (err) return err;
    set_bit(port->cmd, 0);

    return 0;
}


static int ahci_port_alloc_buffers(struct sata_device *dev, struct hba_port *port)
{
    struct page *clb_page = palloc(0, PALLOC_ZERO);
    if (!clb_page)
        return -1;

    paddr_t clb = pn_to_paddr(clb_page->pfn);
    port->clb64 = clb;
    dev->clb_phys = clb;
    dev->clb_virt = (void*)p_to_v(clb);

    struct page *fb_page  = palloc(0, PALLOC_ZERO);
    if (!fb_page) {
        pfree(clb_page);
        return -1;
    }

    paddr_t fb = pn_to_paddr(fb_page->pfn);
    port->fb64 = fb;
    dev->fb_phys = fb;
    dev->fb_virt = (void*)p_to_v(fb);

    return 0;
}


static int ahci_submit_command(struct sata_device *dev, u8 slot)
{
    struct hba_port *port = dev->port;

    paddr_t table_phys = pn_to_paddr(dev->dma_table->pfn);
    struct ahci_cmd_header *cmd = (struct ahci_cmd_header *)(dev->clb_virt) + slot;
    cmd->ctba64 = table_phys;

    port->serr = (u32)~0U;
    port->is = (u32)~0U;

    if (ahci_wait_port(port))
        return -ETIMEDOUT;

    set_bit(port->ci, slot);

    u64 deadline = timer_get_ms() + AHCI_CMD_TIMEOUT_MS;
    while (get_bit(port->ci, slot)) {
        if (get_bit(port->is, 30))
            return -EIO;
        if (timer_get_ms() >= deadline)
            return -ETIMEDOUT;
    }

    if (get_bit(port->tfd, 0))
        return -EIO;

    return 0;
}


static int ahci_fetch_device_geometry(struct sata_device *dev)
{
    struct hba_port *port = dev->port;

    struct page *table_page = dev->dma_table;
    struct page *buf_page = dev->dma_buffer;

    struct ahci_cmd_table *table = (struct ahci_cmd_table *)p_to_v(pn_to_paddr(table_page->pfn));
    paddr_t buf_phys = pn_to_paddr(buf_page->pfn);
    void *buf        = (void *)p_to_v(buf_phys);

    struct ahci_cmd_header *cmd = (struct ahci_cmd_header *)dev->clb_virt;
    cmd->cfl = sizeof(struct fis_reg_h2d) / sizeof(u32);
    cmd->w = 0;
    cmd->prdtl = 1;
    cmd->prdbc = 0;

    struct fis_reg_h2d *fis = (struct fis_reg_h2d *)table->cfis;
    fis->fis_type = 0x27;
    fis->pmport_c = (1U << 7);
    fis->command = 0xEC;
    fis->device = 0;

    table->prdt[0].dba64 = buf_phys;
    table->prdt[0].dbc = (512 - 1) | (1U << 31);

    int err = ahci_submit_command(dev, 0);
    if (err)
        return err;

    u16 *data = (u16*)buf;

    if (get_bit(data[83], 10)) {
        dev->total_sectors = (u64)data[100]
            | ((u64)data[101] << 16)
            | ((u64)data[102] << 32)
            | ((u64)data[103] << 48); 
        dev->lba48 = true;
    }
    else {
        dev->total_sectors = (u32)data[60] | ((u32)data[61] << 16);
        dev->lba48 = false;
    }

    if ((data[106] & 0xC000) == 0x4000 && get_bit(data[106], 12))
        dev->sector_size = ((u32)data[117] | ((u32)data[118] << 16)) * 2;
    else
        dev->sector_size = 512;
    
    return 0;
}


static int sata_block_read(struct block_device *dev, u64 lba, u32 count, void *buf)
{
    struct sata_device *sata = (struct sata_device *)dev->private;
    struct hba_port *port = sata->port;

    struct page *table_page = sata->dma_table;
    struct page *buf_page   = sata->dma_buffer;

    struct ahci_cmd_table *table = (struct ahci_cmd_table *)p_to_v(pn_to_paddr(table_page->pfn));
    paddr_t buf_phys = pn_to_paddr(buf_page->pfn);
    void *dma_buffer = (void *)p_to_v(buf_phys);

    if (ahci_wait_port(port))
        return -ETIMEDOUT;

    struct ahci_cmd_header *cmd = (struct ahci_cmd_header *)sata->clb_virt;
    cmd->cfl = sizeof(struct fis_reg_h2d) / sizeof(u32);
    cmd->w = 0;
    cmd->prdtl = 1;

    u32 max_sectors = ((1ULL << AHCI_DMA_BUFFER_PAGE_ORDER) * PAGE_SIZE) / sata->sector_size;
    struct fis_reg_h2d *fis = (struct fis_reg_h2d *)table->cfis;

    while (count > 0) {
        u16 chunk = (count > max_sectors) ? (u16)max_sectors : count;

        cmd->prdbc = 0;

        fis->fis_type = 0x27;
        fis->pmport_c = (1U << 7);
        fis->device = (1U << 6);
        if (sata->lba48) {
            fis->command   = 0x25;
            fis->device    = (1U << 6);
            fis->lba0      = (u8)(lba & 0xFF);
            fis->lba1      = (u8)((lba >> 8) & 0xFF);
            fis->lba2      = (u8)((lba >> 16) & 0xFF);
            fis->lba3      = (u8)((lba >> 24) & 0xFF);
            fis->lba4      = (u8)((lba >> 32) & 0xFF);
            fis->lba5      = (u8)((lba >> 40) & 0xFF);
            fis->count_lo  = (u8)(chunk & 0xFF);
            fis->count_hi  = (u8)((chunk >> 8) & 0xFF);
        } else {
            fis->command   = 0xC8;
            fis->device    = (1U << 6) | (u8)((lba >> 24) & 0x0F);
            fis->lba0      = (u8)(lba & 0xFF);
            fis->lba1      = (u8)((lba >> 8) & 0xFF);
            fis->lba2      = (u8)((lba >> 16) & 0xFF);
            fis->lba3      = 0;
            fis->lba4      = 0;
            fis->lba5      = 0;
            fis->count_lo  = (u8)(chunk & 0xFF);
            fis->count_hi  = 0;
        }

        fis->count_lo = (u8)(chunk & 0xFF);
        fis->count_hi = (u8)((chunk >> 8) & 0xFF);

        u32 bytes = (u32)chunk * sata->sector_size;
        table->prdt[0].dba64 = buf_phys;
        table->prdt[0].dbc = (bytes - 1) | (1U << 31);

        int err = ahci_submit_command(sata, 0);
        if (err)
            return err;

        memcpy(buf, dma_buffer, bytes);

        buf        += bytes;
        lba        += chunk;
        count      -= chunk;
    }

    return 0;
}


static int sata_block_write(struct block_device *dev, u64 lba, u32 count, const void *buf)
{

}


static int sata_block_flush(struct block_device *dev)
{

}


static struct block_device_ops sata_ops = {
    .read_sector = sata_block_read,
    .write_sector = sata_block_write,
    .flush = sata_block_flush
};


static int ahci_register_block_device(struct sata_device *dev)
{
    dev->block = (struct block_device) {
        .name = dev->name,
        .ops = &sata_ops,
        .sector_count = dev->total_sectors,
        .sector_size = dev->sector_size,
        .private = dev
    };

    return block_register(&dev->block);
}


static int ahci_init_sata_device(struct sata_device *dev, struct hba_port *port)
{
    dev->port = port;
    dev->total_sectors = 0;
    dev->sector_size   = 512; 

    int err = ahci_port_stop(port);
    if (err) return err;

    if (ahci_port_alloc_buffers(dev, port))
        return -ENOMEM;

    err = ahci_port_start(port);
    if (err) return err;
    
    dev->dma_table = palloc(0, PALLOC_ZERO);
    if (!dev->dma_table) 
        return -ENOMEM;

    dev->dma_buffer = palloc(AHCI_DMA_BUFFER_PAGE_ORDER, PALLOC_ZERO);
    if (!dev->dma_buffer) {
        pfree(dev->dma_table);
        return -ENOMEM;
    }

    err = ahci_fetch_device_geometry(dev);
    if (err) return err;

    sprintf(dev->name, "sata%u", sata_device_count);

    if (port->sig == AHCI_SATA_DRIVE)
        ahci_register_block_device(dev);

    sata_device_count++;
    list_add(&dev->list, &sata_list);

    return 0;
}


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

    struct vm_area *area = vmap(&kernel_space, abar, VASL_MMIO_BASE, PAGE_ALIGN_UP(sizeof(struct hba_mem)), PAGE_FLAG_READWRITE | PAGE_FLAG_NOCACHE);
    controller = (struct hba_mem *)area->virt_start;

    controller->ghc |= (1U << 31);

    if (controller->cap2 & 1) {
        set_bit(controller->bohc, 1);
        u64 deadline = timer_get_ms() + AHCI_BOHC_TIMEOUT_MS;
        while (get_bit(controller->bohc, 0)) {
            if (timer_get_ms() >= deadline)
                break;
        }
    }

    for (int i = 0; i < 32; ++i) {
        if (get_bit(controller->pi, i)) {
            struct hba_port *port = &controller->ports[i];

            u8 det = port->ssts & 0x0F;
            u8 ipm = (port->ssts >> 8) & 0x0F;

            if (det == 3 && ipm == 1) {
                struct sata_device *dev = (struct sata_device *)kmalloc(sizeof(struct sata_device), PAL_KERNEL);
                dev->port_num = i;

                if (ahci_init_sata_device(dev, port)) {
                    kfree(dev);
                    continue;
                }
            }
        }
    }
}
