#!/bin/bash

MESON_OPTIONALS=""
MESON_CROSS_INI=""

OUTDIR="./bin"
OSNAME="tinyos"

if [ "${1-}" == "--clean" ]; then
    MESON_OPTIONALS="--wipe"
fi

case "$(uname -s)" in 
    Linux)  MESON_CROSS_INI="linux-x86.ini" ;;
    Darwin) MESON_CROSS_INI="macos-ARM.ini" ;;
esac

# Configure and build kernel
meson setup $MESON_OPTIONALS build/ --cross-file $MESON_CROSS_INI --prefix /kernel
ninja -C build/

cp build/src/kernel/kernel.elf bin/kernel/
cp build/src/boot/bootx64.efi bin/boot/

dd if=/dev/zero of=$OUTDIR/$OSNAME.img bs=512 count=93750

mformat -i $OUTDIR/$OSNAME.img ::
mmd -i $OUTDIR/$OSNAME.img ::/efi
mmd -i $OUTDIR/$OSNAME.img ::/efi/boot
mmd -i $OUTDIR/$OSNAME.img ::/bin 
mcopy -i $OUTDIR/$OSNAME.img $OUTDIR/boot/bootx64.efi ::/efi/boot
mcopy -i $OUTDIR/$OSNAME.img $OUTDIR/kernel/kernel.elf ::/bin/