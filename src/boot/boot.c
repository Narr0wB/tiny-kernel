
#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <elf.h>

#include "efidef.h"
#include <utils.h>

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

EFI_STATUS EFIAPI Halt() {
    UINTN event_index;
    EFI_STATUS status = uefi_call_wrapper(BS->WaitForEvent, 3, 1, &ST->ConIn->WaitForKey, event_index);
    return status;
}

EFI_STATUS EFIAPI InitializeGraphics(
    OUT framebuffer_t *framebuffer
) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Graphics;
    EFI_STATUS s;

    s = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Graphics);
    if (EFI_ERROR(s)) {
        Print(L"Unable to locate the specified protocol\n");
        return s;
    }

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
    EFI_STATUS s;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
    
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFS;
    uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&SimpleFS);

    if (Directory == NULL) {
        uefi_call_wrapper(SimpleFS->OpenVolume, 2, SimpleFS, &Directory);
    }

    s = uefi_call_wrapper(Directory->Open, 5, Directory, File, FilePath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (s != EFI_SUCCESS) {
        Print(L"Failed to open the path on the current image: %s\n", FilePath);
        return s;
    }

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE ImageHandle, 
    EFI_SYSTEM_TABLE *SystemTable
) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"Loading the memory map...\n");

    // Load the memory map
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN                  MemoryMapSize;
    UINTN                  MemoryMapKey;
    UINTN                  DescriptorSize;
    UINT32                 DescriptorVersion;
    EFI_STATUS             status;

    MemoryMapSize = 0;
    MemoryMap     = NULL;

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MemoryMapKey, &DescriptorSize, &DescriptorVersion);
    if (status != EFI_BUFFER_TOO_SMALL) { Print(L"Failed to get info about the memory map! status: %d\n", status); Halt(); }

    MemoryMapSize += 2 * DescriptorSize;
    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MemoryMapSize, (void**)&MemoryMap);
    if (status != EFI_SUCCESS) { Print(L"Failed to allocate memory for the memory map!\n"); Halt(); }

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MemoryMapKey, &DescriptorSize, &DescriptorVersion);
    if (status != EFI_SUCCESS) { Print(L"Failed to load the memory map! status: %d\n", MemoryMapSize); Halt(); }

    Print(L"Loading kernel...\n");

    EFI_FILE *KernelELF;
    EFI_STATUS s = 0;

    s = LoadFile(ImageHandle, SystemTable, NULL, L"\\bin\\kernel.elf", &KernelELF);

    if (s != EFI_SUCCESS) {
        return EFI_NOT_FOUND;
    }

    Elf64_Ehdr header;
    Print(L"Found kernel.elf file! Parsing header...\n");

    // Load the elf file header
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO);
    UINTN header_size = sizeof(header);

    EFI_FILE_INFO *FileInfo;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    uefi_call_wrapper(KernelELF->GetInfo, 4, KernelELF, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);
    uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &header_size, (void*)&header);

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
    uefi_call_wrapper(KernelELF->SetPosition, 2, KernelELF, header.e_phoff);
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, size, (void**)&ProgramHeaders);
    uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &size, (void*)ProgramHeaders);
    
    int requested_pages = 0;
    for (UINTN i = 0; i < header.e_phnum; ++i) {
        Elf64_Phdr pHeader = ProgramHeaders[i];
        if (pHeader.p_type == PT_LOAD) {
            requested_pages += (pHeader.p_memsz + 0x1000 - 1) / 0x1000;
        }
    }

    paddr_t kernel_load = 0;
    size_t kernel_pages = 0;
  
    for (UINTN i = 0; i < header.e_phnum; ++i) {
        Elf64_Phdr pHeader = ProgramHeaders[i];
        
        // For each program header, find the number of pages necessary to load the program into memory
        // then allocate the pages at the address specified by the program header, and finally copy data 
        // at given address
        switch (pHeader.p_type) {
            case PT_LOAD: {
                int pages = (pHeader.p_memsz + 0x1000 - 1) / 0x1000;
                Elf64_Addr mSegment = pHeader.p_paddr;
                uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &mSegment);
                uefi_call_wrapper(KernelELF->SetPosition, 2, KernelELF, pHeader.p_offset);
                UINTN size = pHeader.p_filesz;
                uefi_call_wrapper(KernelELF->Read, 3, KernelELF, &size, (void*)mSegment);
                Print(L"Loading segment at addr %p\n", mSegment);

                if (kernel_load == 0) {
                    kernel_load = mSegment;
                }

                kernel_pages += pages;
                break;
            }
        } 
    }
    
    // Allocate memory for all the variables that we need to pass to our kernel
    bootinfo_t *BootInfo = NULL; 
    UINTN kvPages = (sizeof(bootinfo_t) + 0x1000 - 1) / 0x1000;
    uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, kvPages, &BootInfo);

    Print(L"Kernel successfully loaded!\n");

    framebuffer_t *framebuffer = &BootInfo->framebuffer;
    s = InitializeGraphics(framebuffer);

    BootInfo->map.map = (memory_descriptor_t*)MemoryMap;
    BootInfo->map.size = (MemoryMapSize / DescriptorSize);
    BootInfo->kernel_load = kernel_load;
    BootInfo->kernel_pages = kernel_pages;

    uefi_call_wrapper(BS->FreePool, 1, FileInfo);
    uefi_call_wrapper(BS->FreePool, 1, ProgramHeaders);

    uefi_call_wrapper(BS->ExitBootServices, ImageHandle, MemoryMapKey);

    // Declare and call the kernel entry point;
    int (*_kernel_entry)(bootinfo_t*) = ( (__attribute__((sysv_abi)) int(*)(bootinfo_t*)) (header.e_entry) );
    int code = _kernel_entry(BootInfo);

    __builtin_unreachable();
    
    return EFI_SUCCESS; 
}


