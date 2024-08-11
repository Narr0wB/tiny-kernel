OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

all:
	nasm -g -f elf64 -o inta.o int.asm
	nasm -g -f elf64 -o isra.o isr.asm
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c int.c -o int.o
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c isr.c -o isr.o
	$(LD) -r -o $(OUTDIR)/int.o inta.o int.o
	$(LD) -r -o $(OUTDIR)/isr.o isra.o isr.o
	rm int.o 
	rm isr.o 
	rm inta.o
	rm isra.o
