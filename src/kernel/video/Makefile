OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

CC = gcc 
KERNEL_CFLAGS = -I$(INCLUDE) -ffreestanding -fshort-wchar -Wall -Wpedantic

all:
	$(CC) $(KERNEL_CFLAGS) -c video.c -o $(OUTDIR)/video.o 
	
