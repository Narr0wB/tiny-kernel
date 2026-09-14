
#include <arch/x86/cpu.h>
#include <arch/x86/idt.h>
#include <arch/x86/timer.h>
#include <arch/x86/drivers/pic.h>
#include <arch/x86/mm/paging.h>

#include <tiny/boot/boot.h>
#include <tiny/boot/efi.h>
#include <tiny/video.h>
#include <tiny/tty/tty.h>
#include <tiny/io.h>
#include <tiny/serial.h>
#include <tiny/mm/palloc.h>
#include <tiny/mm/vasl.h>
#include <tiny/mm/bootmem.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/panic.h>
#include <tiny/fs/vfs.h>
#include <tiny/fs/ramfs.h>
#include <tiny/fs/namei.h>
#include <tiny/platform/acpi.h>
#include <tiny/platform/pci.h>
#include <tiny/platform/ahci.h>

void clean_efi_memory_map(struct efi_memory_map *mmap) 
{
    struct efi_memory_descriptor *last_free = NULL;
    size_t mmap_descriptors = 0;

    for (size_t i = 0; i < mmap->size; ++i) {
        struct efi_memory_descriptor *current = &mmap->map[i];

        if (current->type == EFI_CONVENTIONAL_MEMORY
            || current->type == EFI_BOOT_SERVICES_CODE
            || current->type == EFI_BOOT_SERVICES_DATA) 
        {
            if (!last_free 
                || current->phys_start != last_free->phys_start + last_free->npages * PAGE_SIZE) 
            {
                last_free = &mmap->map[mmap_descriptors++];
                *last_free = *current;
                last_free->type = EFI_CONVENTIONAL_MEMORY;
                continue;
            }
            else {
                last_free->npages += current->npages;
            }
        }
        else {
            mmap->map[mmap_descriptors++] = *current; 
        }

    }
    
    mmap->size = mmap_descriptors;
}

void print_efi_memory_map(struct efi_memory_map *mmap) {
    kprintf(KERN_NONE, EOL);
    for (size_t i = 0; i < mmap->size; ++i) {
        struct efi_memory_descriptor *current = &(mmap->map[i]);
        kprintf(KERN_DEBUG, "DESC. NO: %d TYPE: %d PHYS_START: %p NPAGES: %d"EOL, i+1, current->type, current->phys_start, current->npages);
    }
}

// In init_data we have the physical address that points to the bootinfo structure given to us by the UEFI bootloader
__attribute__((aligned(4096))) int _kentry(struct bootinfo *init_data) {
    cpu_init_boot();
    init_serial();
    init_idt();
    init_pic();
    init_timer();

    kprintf(KERN_INFO, "Initializing memory...\n");
    kprintf(KERN_INFO, "Kernel loaded at (paddr) %p - (paddr) %p\n", init_data->kernel_image_start, init_data->kernel_image_end);

    clean_efi_memory_map(&init_data->map);
    // print_efi_memory_map(&init_data->map);

    init_bootmem(init_data);

    struct memory_info info = {0};
    bootmem_get_memory_info(&info);

    init_paging(&info);
    init_palloc(&info);
    init_vmm();
    cpu_setup_main_core();
    init_video(&init_data->framebuffer);
    init_tty();
    init_acpi(init_data->rsdp);
    init_pci();
    init_ahci();

    struct block_device *dev = block_find(1);
    kprintf(KERN_DEBUG, "dev %p"EOL, dev);
    if (dev) {
        char buffer[512];
        int err = block_read(dev, 0, 1, (void*)buffer);
        if (err)
            kprintf(KERN_ERROR, "Could not read from block device!"EOL);
        else
            kprintf(KERN_DEBUG, "Sata sector 0: %s"EOL, buffer);
    }


    init_vfs();
    init_ramfs();

    // struct path p = {0};
    // int err = path_walk(NULL, "/tmp", &p);

    // struct qstr name = QSTR("testfile");
    // struct dentry *entry = dalloc(p.dentry, &name);
    // vfs_create(p.dentry->inode, entry, 0);

    // kprintf(KERN_DEBUG, "Create testfile dentry %p"EOL, entry);

    // err = path_walk(NULL, "/tmp/testfile", &p);
    // struct file *f;
    // vfs_open(&p, &f, 0);

    // kprintf(KERN_DEBUG, "Successfully retrieved path %i %p, dir: %i"EOL, err, p.dentry, S_ISDIR(p.dentry->inode->mode));

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
