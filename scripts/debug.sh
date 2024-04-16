#!/bin/bash

set -e

QEMU_CMD=$(echo qemu-system-x86_64 \
    -cpu qemu64 \
    -net none \
    -drive if=pflash,format=raw,unit=0,file=/usr/share/OVMF/OVMF_CODE.fd,readonly=on \
    -drive if=pflash,format=raw,unit=1,file=/usr/share/OVMF/OVMF_VARS.fd \
    -drive id=sysimg,if=none,format=raw,file=/vm/helium.img \
    -device ahci,id=ahci \
    -device ide-hd,drive=sysimg,bus=ahci.0 \
    -debugcon stdio \
    -display none \
    -m $MEMORY \
    -smp $CPU \
    -d int \
    -s -S \
    -monitor unix:qms,server)

tmux new sh -c "sleep 1 && rust-gdb -tui" \; \
    splitp -h $QEMU_CMD \; \
    splitp -v sh -c "sleep 1 && socat -,echo=0,icanon=0 unix-connect:qms"