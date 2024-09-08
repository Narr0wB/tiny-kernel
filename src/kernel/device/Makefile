all:
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c ps2.c -o $(OUTDIR)/kernel/ps2.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c keyboard.c -o $(OUTDIR)/kernel/keyboard.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c device.c -o $(OUTDIR)/kernel/device.o 
	$(CC) $(KERNEL_CFLAGS) -I$(KERNEL_INCLUDE) -c serial.c -o $(OUTDIR)/kernel/serial.o 
	
