
#include <arch/cpu.h>
#include <arch/idt.h>

#include <tiny/boot/boot.h>
#include <tiny/video.h>
#include <tiny/tty/tty.h>
#include <tiny/io.h>
#include <tiny/serial.h>
#include <tiny/mm/memory.h>
#include <tiny/mm/palloc.h>
#include <tiny/mm/vasl.h>
#include <tiny/mm/bootmem.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/device/device.h>
#include <tiny/panic.h>

// This function will merge memory descriptors of type EfiBootServicesData/Code with EfiConventionalMemory descriptors
void clean_efi_memory_map(struct efi_memory_map *mmap) {
    // The first descriptor that classifies as "free" 
    struct efi_memory_descriptor *current_free = NULL;
    size_t mmap_descriptors = 0;
    bool is_current_free = false;

    for (size_t i = 0; i < mmap->size; ++i) {
        struct efi_memory_descriptor *current = &mmap->map[i];
        
        // If there are any memory segements between the start of the kernel image and the end of the identity paging then its kernel memory, do not touch.
        // if (current->phys_start < _mem_info.kernel_image_end && current->phys_start > _mem_info.kernel_image_start) {
        //     current->type = EFI_LOADER_CODE;

        //     is_current_free = false;
        //     mmap->map[mmap_descriptors] = *current; 
        //     mmap_descriptors++;

        //     continue;
        // };

        // Ignore, residuals from firmware
        if (current->type == EFI_RESERVED_MEMORY_TYPE) continue; 
        
        if (current->type == EFI_CONVENTIONAL_MEMORY 
            || current->type == EFI_BOOT_SERVICES_CODE 
            || current->type == EFI_BOOT_SERVICES_DATA 
            || current->type == EFI_RUNTIME_SERVICES_CODE
            || current->type == EFI_RUNTIME_SERVICES_DATA) 
        {
            if (is_current_free && current->phys_start != current_free->phys_start + current_free->npages * PAGE_SIZE) {
                is_current_free = false;
            }


            if (!is_current_free) {
                current_free = &mmap->map[mmap_descriptors];
                *current_free = *current;
                current_free->type = EFI_CONVENTIONAL_MEMORY;

                is_current_free = true;

                mmap_descriptors++;
                continue;
            }
            else {
                current_free->npages += current->npages;
            }
        }
        else {
            is_current_free = false;
            mmap->map[mmap_descriptors] = *current; 
            mmap_descriptors++;
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

    kprintf(KERN_INFO, "Initializing memory...\n");
    kprintf(KERN_INFO, "Kernel loaded at (paddr) %p - (paddr) %p\n", init_data->kernel_image_start, init_data->kernel_image_end);

    clean_efi_memory_map(&init_data->map);
    print_efi_memory_map(&init_data->map);

    bootmem_init(init_data);

    for (size_t i = 0; i < init_data->map.size; ++i) {
        struct efi_memory_descriptor *desc = &(init_data->map.map[i]);
        switch (desc->type) {
            case EFI_CONVENTIONAL_MEMORY: bootmem_insert_region(desc->phys_start, desc->phys_start + desc->npages * PAGE_SIZE, REGION_TYPE_RAM); break;
            case EFI_MEMORY_MAPPED_IO: bootmem_insert_region(desc->phys_start, desc->phys_start + desc->npages * PAGE_SIZE, REGION_TYPE_MMIO); break;
        }
    }

    struct memory_info info = {0};
    bootmem_get_memory_info(&info);

    setup_kernel_paging(&info);
    init_palloc(&info);

    cpu_setup_main_core();

    // extern struct buddy_allocator allocator;
    // kprintf(KERN_DEBUG, "free_pages: %ld\n", allocator.free_pages);
    // kprintf(KERN_DEBUG, "page_count: %ld\n", allocator.page_count);
    // kprintf(KERN_DEBUG, "Maps:\n");
    // for (int i = 0; i < allocator.map_count; ++i) {
    //     kprintf(KERN_DEBUG, "map of order %d: %ld\n", i, allocator.maps[i].free_count);
    // }

    // struct page *p[100];
    // for (int i = 0; i < 100; ++i) {
    //     p[i] = palloc(3, 0);
    // }

    // kprintf(KERN_DEBUG, "free_pages: %ld\n", allocator.free_pages);
    // kprintf(KERN_DEBUG, "page_count: %ld\n", allocator.page_count);
    // kprintf(KERN_DEBUG, "Maps:\n");
    // for (int i = 0; i < allocator.map_count; ++i) {
    //     kprintf(KERN_DEBUG, "map of order %d: %ld\n", i, allocator.maps[i].free_count);
    // }

    // for (int i = 0; i < 100; ++i) {
    //     pfree(p[i], 3);
    // }
    // // kprintf(KERN_DEBUG, "Here is the page i got, \ncount: %d, \npnumber: %lx, \norder: %d, \nflags: %d, virt: %p\n", atomic_get(&p->count), p->pnumber, p->order, p->flags, p->virt);

    // kprintf(KERN_DEBUG, "free_pages: %ld\n", allocator.free_pages);
    // kprintf(KERN_DEBUG, "page_count: %ld\n", allocator.page_count);
    // kprintf(KERN_DEBUG, "Maps:\n");
    // for (int i = 0; i < allocator.map_count; ++i) {
    //     kprintf(KERN_DEBUG, "map of order %d: %ld\n", i, allocator.maps[i].free_count);
    // }

    init_video(&init_data->framebuffer);
    init_tty();
    init_device();

    panic("eddu");

    // __asm__ volatile (
    //     "int $0x40"
    // );

    kprintf(KERN_DEBUG, "I survived the interrupt, wow!!!\n");

    // kprintf(KERN_INFO, "Initializing the vga and tty module...");
    
    // kprintf("Initializing IDT and setting up interrupt service routines...");
    // init_idt();
    // kprintf("DONE\n");

    // kprintf("Initializing input devices...");
    // init_device();
    // kprintf("DONE\n");

    // extern struct memory_info _mem_info;

    // int test = 0;
    // kprintf("[INFO] Working on stack %p"EOL, &test);
    // kprintf("[INFO] Available Physical RAM: %d pages, %dM"EOL, _mem_info.available_phys_pages, (_mem_info.available_phys_pages * PAGE_SIZE) / (1024 * 1024));
    // kprintf("[INFO] eddu %p"EOL, init_data->framebuffer.base_addr);

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}