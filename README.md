# tiny-kernel

A custom bare-bones kernel for the x86-64 architecture 

## Get started

To build this project, you will need the following tools:

- make
- meson
- ninja
- mtools

Once gathered all the necessary tools, you can leverage the `./build.sh` script in the root directory to build both the kernel and the bootloader and package them into a single ISO image ready to be put on a bootable device:

```bash
❯ ./build.sh --help
Usage: ./build.sh [--clean|-c] [--suppress-warnings|-s]
```

## Running the kernel on an Emulator 

If you want to run the kernel on an emulator (such as QEMU), install `qemu-system-x86_64` and then use the provided `./run-qemu.sh` script in the root direcotry as follows:

```sh
❯ ./run-qemu.sh
```