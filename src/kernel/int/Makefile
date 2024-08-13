OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

all:
	nasm -g -f elf64 -o tmp.int.asm.o int.asm
	nasm -g -f elf64 -o tmp.isr.asm.o isr.asm
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c int.c -o tmp.int.c.o
	$(LD) -r -o $(OUTDIR)/int.o tmp.int.asm.o tmp.int.c.o tmp.isr.asm.o
	rm tmp.*
