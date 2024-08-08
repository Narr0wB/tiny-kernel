SRCDIR = src
OUTDIR = bin

CC = gcc 
BOOTLOADER_CFLAGS = -I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -Wall  
KERNEL_CFLAGS = -ffreestanding -fshort-wchar -Wall -Wpedantic

LD = ld
BOOTLOADER_LDFLAGS = -shared -Bsymbolic -Lgnu-efi/ -Tgnu-efi/elf_x86_64_efi.lds gnu-efi/crt0-efi-x86_64.o -nostdlib
KERNEL_LDFLAGS = -T src/kernel/kernel.ld  -nostdlib 

OBJCOPY = objcopy

BOOT = bootx64
KERNEL = kernel
OSNAME = tinyos

KSRCS := $(wildcard $(SRCDIR)/kernel/*.c)
OBJS = $(patsubst $(SRCDIR)/kernel/%.c, $(OUTDIR)/kernel/%.o, $(KSRCS))

# Build the kernel source files
$(OUTDIR)/kernel/%.o: $(SRCDIR)/kernel/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -c $^ -o $@

# Link the kernel obj files into one elf executable
$(KERNEL).elf: $(OBJS)
	$(LD) $(KERNEL_LDFLAGS) -o $(OUTDIR)/kernel/$(KERNEL).elf $^

# Create the efi application
$(BOOT): $(SRCDIR)/boot/*.c
	$(CC) $(BOOTLOADER_CFLAGS) -c $^ -o $(OUTDIR)/boot/$(BOOT).o
	$(LD) $(BOOTLOADER_LDFLAGS) $(OUTDIR)/boot/$(BOOT).o -o $(OUTDIR)/boot/$(BOOT).so -lgnuefi -lefi 
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(OUTDIR)/boot/$(BOOT).so $(OUTDIR)/boot/$(BOOT).efi

# Create the image file and copy the efi application (bootloader) to the efi partition of the image
buildimg: $(BOOT) $(KERNEL).elf
	dd if=/dev/zero of=$(OUTDIR)/$(OSNAME).img bs=512 count=93750
	mformat -i $(OUTDIR)/$(OSNAME).img ::
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi/boot
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/boot/$(BOOT).efi ::/efi/boot
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/kernel/$(KERNEL).elf ::

run:
	qemu-system-x86_64 -cpu qemu64 -bios /usr/share/edk2-ovmf/x64/OVMF.fd -drive file=$(OUTDIR)/$(OSNAME).img,if=ide
