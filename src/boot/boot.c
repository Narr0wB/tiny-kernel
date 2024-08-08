
#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <elf.h>

#include "efidef.h"
#include "utils.h"

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
    OUT framebuffer_t *framebuffer
) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Graphics;
    EFI_STATUS s;

    s = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Graphics);
    if (EFI_ERROR(s)) {
        Print(L"Unable to locate the specified protocol\n");
        return s;
    }

    framebuffer-> 

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

    Print(L"Loading kernel...\n");

    EFI_FILE *KernelELF;
    EFI_STATUS s = 0;

    s = LoadFile(ImageHandle, SystemTable, NULL, L"kernel.elf", &KernelELF);

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

                break;
            }
        } 
    }

    Print(L"Kernel successfully loaded!\n");
    
    EFI_PHYSICAL_ADDRESS  
    s = InitializeGraphics();

    // Declare and call the kernel entry point;
    int (*_kernel_entry)(void) = ( (__attribute__((sysv_abi)) int(*)(void)) header.e_entry );
    int code = _kernel_entry();
    Print(L"Kernel exited with code %x\n", code);
    
    // Control returned to the EFI firmware
    UINTN Key;
    Print(L"Press any key to continue...\n");
    uefi_call_wrapper(BS->WaitForEvent, 3, 1, &SystemTable->ConIn->WaitForKey, &Key);
   
    return EFI_SUCCESS; 
}


