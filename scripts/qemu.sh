#!/bin/bash

set -e

qemu-system-x86_64 \
    -cpu qemu64 \
    -net none \
    -drive if=pflash,format=raw,unit=0,file=/usr/share/OVMF/OVMF_CODE.fd,readonly=on \
    -drive if=pflash,format=raw,unit=1,file=/usr/share/OVMF/OVMF_VARS.fd \
    -drive id=sysimg,if=none,format=raw,file=/vm/helium.img \
    -device ahci,id=ahci \
    -device ide-hd,drive=sysimg,bus=ahci.0 \
    -debugcon stdio \
    -display none \
    -vnc :5900 \
    -m $MEMORY \
    -smp $CPU
