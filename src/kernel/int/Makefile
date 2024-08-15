all:
	nasm -g -f elf64 -o tmp.int.asm.o $(ARCH)/int.asm
	nasm -g -f elf64 -o tmp.isr.asm.o $(ARCH)/isr.asm
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c int.c -o tmp.int.c.o
	$(LD) -r -o $(OUTDIR)/kernel/int.o tmp.int.asm.o tmp.int.c.o tmp.isr.asm.o
	rm tmp.*
