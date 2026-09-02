#!/bin/bash

MESON_OPTIONALS=""
MESON_CROSS_INI=""
NINJA_SUPPRESS_WARNINGS=false
RUN_COMMAND=""

OUTDIR="bin/"
OSNAME="tinyos"

for arg in "$@"; do
    case $arg in
        --clean|-c) MESON_OPTIONALS="--wipe" ;; 
        --suppress-warnings|-s) NINJA_SUPPRESS_WARNINGS=true ;;
        --help|-h) 
            echo "Usage: $0 [--clean|-c] [--suppress-warnings|-s]"
            exit 0
            ;;
        --run|-r)
            RUN_COMMAND="./scripts/run-qemu.sh" ;;
        --rund|--run-debug|-rd)
            RUN_COMMAND="./scripts/run-qemu.sh --debug" ;;
        *) 
            echo "Unknown arg: $arg" 
            exit 1
            ;;
    esac
done

case "$(uname -s)" in 
    Linux)  MESON_CROSS_INI="linux-x86.ini" ;;
    Darwin) MESON_CROSS_INI="macos-ARM.ini" ;;
esac

# Configure and build kernel
meson setup $MESON_OPTIONALS build/ --cross-file $MESON_CROSS_INI --prefix /kernel

if $NINJA_SUPPRESS_WARNINGS; then
    ninja -C build/ 2>&1 | grep -E -i "(error|failed|stop|undefined)" | head -20
else
    ninja -C build/ 
fi

# Prepare the environment for the creation of the image file
mkdir -p bin/kernel
mkdir -p bin/boot

cp build/src/vmtiny.elf bin/kernel/
cp build/uefi-bootloader/bootx64.elf bin/boot/
cp build/uefi-bootloader/bootx64.efi bin/boot/

# Create the final image file
dd if=/dev/zero of=$OUTDIR/$OSNAME.img bs=512 count=93750

mformat -i $OUTDIR/$OSNAME.img ::
mmd -i $OUTDIR/$OSNAME.img ::/efi
mmd -i $OUTDIR/$OSNAME.img ::/efi/boot
mmd -i $OUTDIR/$OSNAME.img ::/bin 
mcopy -i $OUTDIR/$OSNAME.img $OUTDIR/boot/bootx64.efi ::/efi/boot
mcopy -i $OUTDIR/$OSNAME.img $OUTDIR/kernel/vmtiny.elf ::/bin/

if [ "$RUN_COMMAND" ]; then
    $RUN_COMMAND
fi