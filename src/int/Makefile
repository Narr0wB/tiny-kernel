all:
	nasm -g -f elf64 -o tmp.isr.asm.o $(ARCH)/isr.asm
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c int.c -o tmp.int.c.o
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c notifier.c -o tmp.notifier.c.o
	$(LD) -r -o $(OUTDIR)/kernel/int.o tmp.isr.asm.o tmp.int.c.o tmp.notifier.c.o
	rm tmp.*
