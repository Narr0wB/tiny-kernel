
all: 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c kernel.c -o $(OUTDIR)/kernel/kernel.o
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c panic.c -o tmp.panic.c.o
	nasm -g -f elf64 -o tmp.panic.asm.o $(ARCH)/panic.asm
	$(LD) -r -o $(OUTDIR)/kernel/panic.o tmp.panic.asm.o tmp.panic.c.o
	rm tmp.*
