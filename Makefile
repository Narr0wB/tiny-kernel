SRCDIR = src
OUTDIR = bin
ASSETS = assets

export CC = gcc
BOOTLOADER_CFLAGS = -I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -Wall  
export KERNEL_CFLAGS = -ffreestanding -fno-stack-protector -fshort-wchar -fno-stack-check -Wall -Wpedantic -g

export LD = ld
BOOTLOADER_LDFLAGS = -shared -Bsymbolic -Lgnu-efi/ -Tgnu-efi/elf_x86_64_efi.lds gnu-efi/crt0-efi-x86_64.o -nostdlib
KERNEL_LDFLAGS = -T src/kernel/kernel.ld  -nostdlib 

export AS = as

OBJCOPY = objcopy

BOOT = bootx64
KERNEL = kernel
OSNAME = tinyos

KSRCS := $(shell find $(SRCDIR)/kernel -name '*.c')
OBJS = $(addprefix $(OUTDIR)/kernel/, $(patsubst %.c,%.o,$(notdir $(KSRCS))))

setup: 
	@mkdir -p $(dir $(OUTDIR)/boot/)
	@mkdir -p $(dir $(OUTDIR)/kernel/)

# Build the kernel source files and create the font embedding
obj:
	$(CC) $(KERNEL_CFLAGS) -Isrc/kernel/ -c $(SRCDIR)/kernel/kernel.c -o $(OUTDIR)/kernel/kernel.o 
	make -C $(SRCDIR)/kernel/video
	make -C $(SRCDIR)/kernel/klibc
	make -C $(SRCDIR)/kernel/memory
	make -C $(SRCDIR)/kernel/int


# Link the kernel obj files into one elf executable
$(KERNEL).elf: obj 
	$(LD) $(KERNEL_LDFLAGS) -o $(OUTDIR)/kernel/$(KERNEL).elf $(OBJS) 

# Create the efi application
$(BOOT): $(SRCDIR)/boot/*.c
	$(CC) $(BOOTLOADER_CFLAGS) -c $^ -o $(OUTDIR)/boot/$(BOOT).o
	$(LD) $(BOOTLOADER_LDFLAGS) $(OUTDIR)/boot/$(BOOT).o -o $(OUTDIR)/boot/$(BOOT).so -lgnuefi -lefi 
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(OUTDIR)/boot/$(BOOT).so $(OUTDIR)/boot/$(BOOT).efi

# Create the image file and copy the efi application (bootloader) to the efi partition of the image
buildimg: setup $(BOOT) $(KERNEL).elf
	dd if=/dev/zero of=$(OUTDIR)/$(OSNAME).img bs=512 count=93750
	mformat -i $(OUTDIR)/$(OSNAME).img ::
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi/boot
	mmd -i $(OUTDIR)/$(OSNAME).img ::/bin 
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/boot/$(BOOT).efi ::/efi/boot
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/kernel/$(KERNEL).elf ::/bin/

run:
	qemu-system-x86_64 -cpu qemu64 -d int -no-shutdown -no-reboot -bios OVMF.fd -drive file=$(OUTDIR)/$(OSNAME).img,if=ide

debug: 
	qemu-system-x86_64 -cpu qemu64 -bios OVMF.fd -s -S -drive file=$(OUTDIR)/$(OSNAME).img,if=ide & disown
	gdb $(OUTDIR)/kernel/$(KERNEL).elf --eval-command="target remote :1234"
