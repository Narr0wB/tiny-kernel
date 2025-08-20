#!/bin/bash

# Configure and build kernel
meson setup build/kernel --cross-file cross_kernel.ini --prefix /kernel
ninja -C build/kernel

# Configure and build bootloader
meson setup build/boot --cross-file cross_boot.ini --prefix /boot
ninja -C build/boot