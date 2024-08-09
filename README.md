# tiny-kernel

a small kernel for the x86_64 architecture 

## Bootloader

The UEFI Bootloader is responsible for:
- Loading and parsing the ELF file containing the kernel
- Setting up a basic framebuffer
- Obtaining the memory map
- Allocating a chunk of memory for all the variables that we need to pass to the kernel
- Lastly jumping to the kernel entry point.

## Kernel

As soos as we are in the kernel, we must:
- Set up memory mapping according to the memory map
