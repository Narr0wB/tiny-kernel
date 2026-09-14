#ifndef AHCI_H
#define AHCI_H

#include <tiny/types.h>
#include <tiny/compiler.h>
#include <tiny/list.h>
#include <tiny/mm/palloc.h>

#define AHCI_DEV_NULL               0
#define AHCI_DEV_SATA               1
#define AHCI_DEV_SEMB               2
#define AHCI_DEV_PM                 3
#define AHCI_DEV_SATAPI             4

#define AHCI_SATA_DRIVE             0x00000101
#define AHCI_SATAPI_DRIVE           0xEB140101
#define AHCI_SEMB_DRIVE             0xC33C0101
#define AHCI_PM_DRIVE               0x96690101

#define AHCI_PORT_TIMEOUT_MS        1000
#define AHCI_PORT_STOP_TIMEOUT_MS   500
#define AHCI_CMD_TIMEOUT_MS         1000
#define AHCI_BOHC_TIMEOUT_MS        2000

#define AHCI_DMA_BUFFER_PAGE_ORDER  7

struct hba_port {
    union {
        struct {
            volatile u32     clb;
            volatile u32     clbu;
        };
        volatile u64         clba;
        volatile u64         clb64;
    };
    union {
        struct {
            volatile u32     fb;
            volatile u32     fbu;
        };
        volatile u64         fba;
        volatile u64         fb64;
    };
    volatile u32             is;
    volatile u32             ie;
    volatile u32             cmd;
    volatile u32             reserved0;
    volatile u32             tfd;
    volatile u32             sig;
    volatile u32             ssts;
    volatile u32             sctl;
    volatile u32             serr;
    volatile u32             sact;
    volatile u32             ci;
    volatile u32             sntf;
    volatile u32             fbs;
    volatile u32             devslp;
    volatile u32             reserved1[10];
    volatile u32             vendor[4];
} __packed;

struct hba_mem {
    volatile u32     cap;
    volatile u32     ghc;
    volatile u32     is;
    volatile u32     pi;
    volatile u32     vs;
    volatile u32     ccc_ctl;
    volatile u32     ccc_pts;
    volatile u32     em_loc;
    volatile u32     em_ctl;
    volatile u32     cap2;
    volatile u32     bohc;
    volatile u8      reserved[116];
    volatile u8      vendor[96];
    struct hba_port  ports[32];
} __packed;

struct ahci_cmd_header {
    u8               cfl:5;
    u8               a:1;
    u8               w:1;
    u8               p:1;
    u8               r:1;
    u8               b:1;
    u8               c:1;
    u8               rsv0:1;
    u8               pmp:4;
    u16              prdtl;
    volatile u32     prdbc;
    union {
        struct {
            u32      ctba;
            u32      ctbau;
        };
        u64          ctba64;
    };
    u32              rsv1[4];
} __packed;

struct ahci_prdt_entry {
    union {
        struct {
            u32      dba;
            u32      dbau;
        };
        u64          dba64;
    };
    u32              rsv;
    u32              dbc;
} __packed;

struct ahci_cmd_table {
    u8                     cfis[64];
    u8                     acmd[16];
    u8                     rsv[48];
    struct ahci_prdt_entry prdt[1];
} __packed;

struct fis_reg_h2d {
    u8               fis_type;
    u8               pmport_c;
    u8               command;
    u8               features_lo;
    u8               lba0;
    u8               lba1;
    u8               lba2;
    u8               device;
    u8               lba3;
    u8               lba4;
    u8               lba5;
    u8               features_hi;
    u8               count_lo;
    u8               count_hi;
    u8               icc;
    u8               control;
    u8               rsv[4];
} __packed;

struct sata_device {
    u8                  port_num;
    struct hba_port    *port;
    u64                 total_sectors;
    u32                 sector_size;
    void               *clb_virt;
    paddr_t             clb_phys;
    void               *fb_virt;
    paddr_t             fb_phys;
    bool                lba48;
    char                name[24];
    struct block_device block;
    struct page        *dma_table;
    struct page        *dma_buffer;
    struct list_head    list;
};

void init_ahci();

#endif
