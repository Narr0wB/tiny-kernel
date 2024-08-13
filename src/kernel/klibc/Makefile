OUTDIR = ../../../bin/kernel
INCLUDE = ../../kernel

all:
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c lib.c -o $(OUTDIR)/lib.o 
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c io.c -o $(OUTDIR)/io.o 
	$(CC) $(KERNEL_CFLAGS) -I$(INCLUDE) -c string.c -o $(OUTDIR)/string.o 
	
