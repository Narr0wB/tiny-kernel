all:
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c lib.c -o $(OUTDIR)/kernel/lib.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c io.c -o $(OUTDIR)/kernel/io.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c string.c -o $(OUTDIR)/kernel/string.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c panic.c -o tmp.panic.c.o
	nasm -g -f elf64 -o tmp.panic.asm.o $(ARCH)/panic.asm
	$(LD) -r -o $(OUTDIR)/kernel/panic.o tmp.panic.asm.o tmp.panic.c.o
	rm tmp.*
	
