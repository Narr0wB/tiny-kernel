#!/bin/bash

dd if=/dev/zero of=bin/testdisk.img bs=512 count=4
echo "Hello from file! (eddu) ts new" | dd of=bin/testdisk.img conv=notrunc bs=512 count=1