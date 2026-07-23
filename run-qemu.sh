#!/bin/bash

OUTDIR="bin"
OSNAME="tinyos"
DBGR=""

# In case of a debug run, preconfigure the debugger to be the correct one for the current platform
case "$(uname -s)" in
    Linux)  DBGR="gdb" ;;
    Darwin) DBGR="x86_64-elf-gdb" ;; 
esac

qemu_args=(
    -m 2G
    -cpu qemu64
    -d int
    -no-shutdown -no-reboot
    -drive if=pflash,file=emulator/uefi-firmware/OVMF_CODE.fd,format=raw,unit=0,readonly=on
    -drive if=pflash,file=emulator/uefi-firmware/OVMF_VARS.fd,format=raw,unit=1
    -drive file=$OUTDIR/$OSNAME.img,if=ide,format=raw
)

if [ "$1" = "--debug" ] || [ "$1" = "-d" ]; then 
    qemu_args+=(
        -gdb tcp::1234,wait=on
    )

    qemu-system-x86_64 "${qemu_args[@]}" > /dev/null 2>&1 &
    qemu_pid=$!

    for i in {0..100}; do
        if ! kill -0 "$qemu_pid" 2>/dev/null; then
            echo "QEMU Exited"
            pkill qemu
            exit 1
        fi

        if nc -z 127.0.0.1 1234 2>/dev/null; then
            break
        fi

        sleep 0.1
    done
    

    $DBGR \
        $OUTDIR/kernel/vmtiny.elf $OUTDIR/boot/bootx64.elf \
        -x emulator/utils.gdb \
        --eval-command="target remote :1234"
else
    qemu-system-x86_64 "${qemu_args[@]}"
fi
