OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

CC = gcc 
KERNEL_CFLAGS = -I$(INCLUDE) -ffreestanding -fshort-wchar -Wall -Wpedantic

all:
	$(CC) $(KERNEL_CFLAGS) -c stdlib.c -o $(OUTDIR)/stdlib.o 
	$(CC) $(KERNEL_CFLAGS) -c stdio.c -o $(OUTDIR)/stdio.o 
	
