
#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <elf.h>

#include "efidef.h"
#include <tiny/boot/boot.h>
#include <tiny/mm/vasl.h>
#include <arch/mm/paging.h>

// TODO: Map the kernel to the higher half of the virtual memory address spaces

EFI_STATUS EFIAPI Halt() {
    UINTN event_index;
    EFI_STATUS status = uefi_call_wrapper(BS->WaitForEvent, 3, 1, &ST->ConIn->WaitForKey, &event_index);
    return status;
}

#define EFI_ERR(expr)                                                       \
    do {                                                                    \
        EFI_STATUS _s = (expr);                                             \
        if (EFI_ERROR(_s)) {                                                \
            Print(L"[ERROR] Call failed at file: %a, line: %d. "            \
                  L"Call: %a, Status code: %r\n",                           \
                  __FILE__, __LINE__, #expr, _s);                           \
            Halt();                                                         \
        }                                                                   \
    } while (0)

int EFIAPI memcmp(
    const void *buf1, 
    const void *buf2, 
    size_t count
) {
    if (!count) return 0;

    while (--count && *(unsigned char*)buf1 == *(unsigned char*)buf2) {
        buf1 = ((unsigned char*)buf1 + 1);
        buf2 = ((unsigned char*)buf2 + 1);
    }

    return *(unsigned char*)buf1 - *(unsigned char*)buf2;
}

EFI_STATUS EFIAPI InitializeGraphics(
    OUT struct framebuffer *framebuffer
) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Graphics = 0;

    EFI_ERR(uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Graphics));

    framebuffer->base_addr    = (void*)Graphics->Mode->FrameBufferBase; 
    framebuffer->size         = (size_t)Graphics->Mode->FrameBufferSize; 
    framebuffer->width        = (uint32_t)Graphics->Mode->Info->HorizontalResolution; 
    framebuffer->height       = (uint32_t)Graphics->Mode->Info->VerticalResolution;
    framebuffer->len_scanline = (uint32_t)Graphics->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
} 

EFI_STATUS EFIAPI LoadFile(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable, 
    IN EFI_FILE *Directory, 
    IN CHAR16 *FilePath, 
    OUT EFI_FILE **File
) {
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_ERR(uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage));

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFS;
    EFI_ERR(uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&SimpleFS));

    if (Directory == NULL) {
        EFI_ERR(uefi_call_wrapper(SimpleFS->OpenVolume, 2, SimpleFS, &Directory));
    }

    EFI_ERR(uefi_call_wrapper(Directory->Open, 5, Directory, File, FilePath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE ImageHandle, 
    EFI_SYSTEM_TABLE *SystemTable
) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Loading kernel...\n");

    EFI_FILE *KernelELF;
    EFI_STATUS status;

    status = LoadFile(ImageHandle, SystemTable, NULL, L"\\bin\\vmtiny.elf", &KernelELF);
    if (status != EFI_SUCCESS) {
        return EFI_NOT_FOUND;
    }

    Print(L"Found kernel.elf file! Parsing header...\n");
    Elf64_Ehdr header;
    UINTN header_size = sizeof(header);
    EFI_ERR(uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &header_size, (void*)&header));

    // Check if the elf file satisfies the necessary condition to run on an x64 machine
    if (
        memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
        header.e_ident[EI_CLASS] != ELFCLASS64                 ||
        header.e_ident[EI_DATA]  != ELFDATA2LSB                ||
        header.e_type            != ET_EXEC                    ||
        header.e_machine         != EM_X86_64                  || 
        header.e_version         != EV_CURRENT
    ) {
        Print(L"Invalid elf format!\n");
        return EFI_ABORTED;
    }

    Print(L"Found valid ELF64 executable, loading program headers...\n");
    
    // Load the program headers
    Elf64_Phdr* ProgramHeaders;
    UINTN size = header.e_phnum * header.e_phentsize;
    EFI_ERR(uefi_call_wrapper(KernelELF->SetPosition, 2, KernelELF, header.e_phoff));
    EFI_ERR(uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, size, (void**)&ProgramHeaders));
    EFI_ERR(uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &size, (void*)ProgramHeaders));

    /*
     *  Now that we have the program headers, we can start with defining the layout in physical memory:
     *  +---------------------------+
     *  | Kernel image              |
     *  | .text / .rodata / .data   |
     *  | .bss                      |
     *  +---------------------------+
     *  | Boot information          |
     *  +---------------------------+
     *  | UEFI memory-map copy      |
     *  +---------------------------+
     *  | Initial page tables       |
     *  +---------------------------+
     *  | Blank page (stack safety) |
     *  +---------------------------+
     *  | Initial kernel stack      |
     *  +---------------------------+
     */
    
    paddr_t kernel_image_start = UINT64_MAX;
    paddr_t kernel_image_end   = 0; 
    size_t kernel_pages        = 0;
    for (UINTN i = 0; i < header.e_phnum; ++i) {
        Elf64_Phdr pHeader = ProgramHeaders[i];
        
        switch (pHeader.p_type) {
            case PT_LOAD: {
                UINTN pages = SIZE_TO_PAGES(pHeader.p_memsz);
                UINTN filesz = pHeader.p_filesz;
                paddr_t segment = pHeader.p_paddr;
                paddr_t segment_end = segment + pages * PAGE_SIZE;

                EFI_ERR(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderCode, pages, (void*)&segment));
                EFI_ERR(uefi_call_wrapper(KernelELF->SetPosition, 2, KernelELF, pHeader.p_offset));
                EFI_ERR(uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &filesz, (void*)segment));

                ZeroMem((void*)(segment + pHeader.p_filesz), pHeader.p_memsz - pHeader.p_filesz);

                kernel_pages += pages;

                if (segment < kernel_image_start)
                    kernel_image_start = segment;
                if (segment_end > kernel_image_end)
                    kernel_image_end = segment_end;

                Print(L"Loading segment at addr %p, size %llx\n", segment, PAGE_SIZE * pages);
                break;
            }

            default: {
                Print(L"Unknown progam header type: %llx\n", pHeader.p_type);
                break;
            }
        } 
    }

    struct bootinfo *BootInfo = (struct bootinfo *)kernel_image_end;
    UINTN bootinfo_pages = SIZE_TO_PAGES(sizeof(struct bootinfo));
    EFI_ERR(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, bootinfo_pages, (EFI_PHYSICAL_ADDRESS*)&BootInfo));
    paddr_t bootinfo_start = (paddr_t)BootInfo;
    paddr_t bootinfo_end   = bootinfo_start + (paddr_t)(bootinfo_pages * PAGE_SIZE);

    struct framebuffer *framebuffer = &BootInfo->framebuffer;
    EFI_ERR(InitializeGraphics(framebuffer));

    EFI_MEMORY_DESCRIPTOR *MemoryMap = (EFI_MEMORY_DESCRIPTOR*)bootinfo_end;
    UINTN                  MemoryMapSize = 0;
    UINTN                  MemoryMapKey;
    UINTN                  DescriptorSize;
    UINT32                 DescriptorVersion;

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, NULL, &MemoryMapKey, &DescriptorSize, &DescriptorVersion);
    if (status != EFI_BUFFER_TOO_SMALL) {
        Print(L"Failed to get info about the memory map! status: %r\n", status);
        Halt();
    }

    // Pad memory map size for safety
    MemoryMapSize += 2 * DescriptorSize;
    UINTN mmap_pages = SIZE_TO_PAGES(MemoryMapSize);

    EFI_ERR(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, mmap_pages, (EFI_PHYSICAL_ADDRESS*)&MemoryMap));
    ZeroMem(MemoryMap, MemoryMapSize);
    EFI_ERR(uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MemoryMapKey, &DescriptorSize, &DescriptorVersion));
    struct efi_memory_map mmap = {
        .map = (struct efi_memory_descriptor *)MemoryMap, 
        .size = (MemoryMapSize/DescriptorSize) + 1
    };

    mmap.map[mmap.size - 1].type       = EfiMemoryMappedIO;
    mmap.map[mmap.size - 1].phys_start = (paddr_t)framebuffer->base_addr;
    mmap.map[mmap.size - 1].npages     = SIZE_TO_PAGES(framebuffer->size);
    mmap.map[mmap.size - 1].attribute  = EFI_MEMORY_UC;

    BootInfo->map                   = mmap;
    BootInfo->kernel_image_start    = kernel_image_start;
    BootInfo->kernel_image_end      = kernel_image_end;

    EFI_ERR(uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MemoryMapKey));

    int (*_kernel_entry)(struct bootinfo *) = ( (__attribute__((sysv_abi)) int(*)(struct bootinfo *)) (V2P(header.e_entry)) );
    int code = _kernel_entry(BootInfo);

    __builtin_unreachable();
    
    return EFI_SUCCESS; 
}