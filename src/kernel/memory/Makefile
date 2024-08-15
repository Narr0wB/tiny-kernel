all:
	nasm -g -f elf64 -o tmp.mem.asm.o $(ARCH)/memory.asm
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c memory.c -o tmp.mem.c.o
	$(LD) -r -o $(OUTDIR)/kernel/memory.o tmp.mem.asm.o tmp.mem.c.o
	rm tmp.*
	
