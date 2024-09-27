#!/bin/bash

set -e

OVMF_CODE=/usr/share/OVMF/OVMF_CODE.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS.fd
SYS_IMG=/vm/helium.iso

QEMU_CMD=$(echo qemu-system-x86_64 \
    -cpu qemu64 \
    -net none \
    -drive if=pflash,format=raw,unit=0,file=${OVMF_CODE},readonly=on \
    -drive if=pflash,format=raw,unit=1,file=${OVMF_VARS} \
    -drive id=sysimg,if=none,format=raw,file=${SYS_IMG} \
    -device ahci,id=ahci \
    -device ide-hd,drive=sysimg,bus=ahci.0 \
    -debugcon stdio \
    -display none \
    -m $MEMORY \
    -smp $CPU \
    -vnc :0\
    -s -S \
    -d int\
    -monitor unix:qms,server)

tmux new sh -c "sleep 1 && rust-gdb -tui" \; \
    splitp -h $QEMU_CMD \; \
    splitp -v sh -c "sleep 1 && socat -,echo=0,icanon=0 unix-connect:qms"