OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

all:
	nasm -g -f elf64 -o mem.o memory.asm
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c memory.c -o memory.o
	$(LD) -r -o $(OUTDIR)/memory.o mem.o memory.o
	rm memory.o 
	rm mem.o
	
