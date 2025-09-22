BASE = $(shell pwd)
UNAME = $(shell uname -s)

export SRCDIR = $(BASE)/src
export OUTDIR = $(BASE)/bin
export ASSETS = $(BASE)/assets
export ARCH = $(BASE)/src/arch/x86_64

ifeq ($(UNAME), Linux)
	export CC = gcc
	export LD = ld
	export AS = as
	export OBJCOPY = objcopy
	export GDB = gdb
endif 

ifeq ($(UNAME), Darwin)
	export CC = x86_64-elf-gcc
	export LD = x86_64-elf-ld
	export AS = x86_64-elf-as
	export OBJCOPY = x86_64-elf-objcopy
	export GDB = x86_64-elf-gdb
endif

export KERNEL_CFLAGS = -ffreestanding -fno-stack-protector -fshort-wchar -fno-stack-check -Wall -Wpedantic -g -O1 
export KERNEL_INCLUDE = $(BASE)/src/kernel/include

BOOTLOADER_LDFLAGS = -shared -Bsymbolic -Lgnu-efi/ -Tgnu-efi/elf_x86_64_efi.lds gnu-efi/crt0-efi-x86_64.o -nostdlib
BOOTLOADER_CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -Wall  
KERNEL_LDFLAGS = -nostdlib

BOOT = bootx64
KERNEL = vmtiny 
OSNAME = tinyos

setup: 
	@mkdir -p $(dir $(OUTDIR)/boot/)
	@mkdir -p $(dir $(OUTDIR)/kernel/)

# Build the kernel source files and create the font embedding
obj:
	make -C $(SRCDIR)/kernel	
	make -C $(SRCDIR)/kernel/video
	make -C $(SRCDIR)/kernel/util
	make -C $(SRCDIR)/kernel/memory
	make -C $(SRCDIR)/kernel/int
	make -C $(SRCDIR)/kernel/tty
	make -C $(SRCDIR)/kernel/device

# Link the kernel obj files into one elf executable
$(KERNEL).elf: obj 
	$(LD) $(KERNEL_LDFLAGS) -T$(ARCH)/kernel.ld -o $(OUTDIR)/kernel/$(KERNEL).elf $(OUTDIR)/kernel/*.o 

# Create the efi application
boot: $(SRCDIR)/boot/*.c
	$(CC) $(BOOTLOADER_CFLAGS) -I$(SRCDIR)/boot/include -c $(SRCDIR)/boot/boot.c -o $(OUTDIR)/boot/$(BOOT).o
	$(CC) $(BOOTLOADER_CFLAGS) -I$(SRCDIR)/boot/include -c $(SRCDIR)/boot/vmap.c -o $(OUTDIR)/boot/vmap.o
	$(LD) $(BOOTLOADER_LDFLAGS) $(OUTDIR)/boot/$(BOOT).o $(OUTDIR)/boot/vmap.o -o $(OUTDIR)/boot/$(BOOT).so -lgnuefi -lefi
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 $(OUTDIR)/boot/$(BOOT).so $(OUTDIR)/boot/$(BOOT).efi

# Create the image file and copy the efi application (bootloader) to the efi partition of the image
buildimg: setup 
	cp build/src/kernel/kernel.elf bin/kernel/
	cp build/src/boot/bootx64.efi bin/boot/
	dd if=/dev/zero of=$(OUTDIR)/$(OSNAME).img bs=512 count=93750
	mformat -i $(OUTDIR)/$(OSNAME).img ::
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi
	mmd -i $(OUTDIR)/$(OSNAME).img ::/efi/boot
	mmd -i $(OUTDIR)/$(OSNAME).img ::/bin 
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/boot/$(BOOT).efi ::/efi/boot
	mcopy -i $(OUTDIR)/$(OSNAME).img $(OUTDIR)/kernel/$(KERNEL).elf ::/bin/

run:
	qemu-system-x86_64 -m 2G -cpu qemu64 -d int -no-shutdown -no-reboot -bios OVMF.fd -drive file=$(OUTDIR)/$(OSNAME).img,if=ide

debug: 
	qemu-system-x86_64 -m 2G -cpu qemu64 -bios OVMF.fd -gdb tcp::1234,wait=on -S -drive file=$(OUTDIR)/$(OSNAME).img,if=ide & sleep 0.2
	$(GDB) $(OUTDIR)/kernel/vmtiny.elf -x utils.gdb --eval-command="target remote :1234"