all:
	nasm -g -f elf64 -o tmp.idt.asm.o $(ARCH)/idt.asm
	nasm -g -f elf64 -o tmp.isr.asm.o $(ARCH)/isr.asm
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c int.c -o tmp.int.c.o
	$(LD) -r -o $(OUTDIR)/kernel/int.o tmp.idt.asm.o tmp.isr.asm.o tmp.int.c.o
	rm tmp.*
