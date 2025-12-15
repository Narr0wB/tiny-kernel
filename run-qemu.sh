#!/bin/bash

OUTDIR="./bin"
OSNAME="tinyos"
GDB=""

# In case of a debug run, preconfigure the debugger to be the correct one for the current platform
case "$(uname -s)" in
    Linux)  GDB="gdb" ;;
    Darwin) GDB="x86_64-elf-gdb" ;; 
esac

if [ "$1" = "--debug" ] || [ "$1" = "-d" ]; then 
	qemu-system-x86_64 -m 2G -cpu qemu64 -bios ./emulator/OVMF.fd -gdb tcp::1234,wait=on -S -drive file=$OUTDIR/$OSNAME.img,if=ide & sleep 0.2
	$GDB $OUTDIR/kernel/vmtiny.elf -x utils.gdb --eval-command="target remote :1234"
else
	qemu-system-x86_64 -m 2G -cpu qemu64 -d int -no-shutdown -no-reboot -bios ./emulator/OVMF.fd -drive file=$OUTDIR/$OSNAME.img,if=ide
fi