#!/bin/sh
# Supposed to run from the docker container!

set -e

LD=x86_64-elf-ld
OBJCOPY=x86_64-elf-objcopy
STRIP=x86_64-elf-strip

cd /build/kernel
cargo build

cp /build/kernel/target/layout-kernel/debug/helium-kernel /build/intermediate/kernel.out

cd /build/userspace
cargo build

cd /build/intermediate

$OBJCOPY --only-keep-debug kernel.out kernel.dbg
$OBJCOPY --strip-debug kernel.out
$STRIP -s -K mmio -K fb -K bootboot -K environment -K initstack kernel.out -o kernel.elf

mkbootimg check kernel.elf

mkdir -p initrd/sys
mkdir -p initrd/bin

cp kernel.elf initrd/sys/kernel.elf
cp font.psf initrd/sys/font.psf

# START FIXME: Temporary until userspace build system is complete
cp /build/userspace/target/layout-userspace/debug/init initrd/bin/init
cp /build/userspace/target/layout-userspace/debug/init /build/out/init
# END

mkbootimg /build/config/bootimg.json helium.iso

cp bootboot.efi /build/out/bootboot.efi
cp kernel.elf /build/out/kernel.elf
cp helium.iso /build/out/helium.iso
cp kernel.dbg /build/out/kernel.dbg