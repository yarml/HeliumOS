#!/bin/sh
# Supposed to run from the docker container!

set -e

LD=x86_64-elf-ld
OBJCOPY=x86_64-elf-objcopy
STRIP=x86_64-elf-strip

cd /build

cargo build

cp /build/target/helium/debug/helium /build/intermediate/helium.out

cd /build/intermediate

$OBJCOPY --only-keep-debug helium.out helium.dbg
$OBJCOPY --strip-debug helium.out
$STRIP -s -K mmio -K fb -K bootboot -K environment -K initstack helium.out -o helium

mkbootimg check helium

mkdir -p initrd/sys
mkdir -p initrd/bin

cp helium initrd/sys/helium

# START FIXME: Temporary until userspace build system is complete
nasm /build/src/test.asm -felf64 -o /build/target/test.o
$LD /build/target/test.o -o /build/target/test
cp /build/target/test initrd/bin/init
cp /build/target/test /build/out/init
# END

mkbootimg /build/config/bootimg.json helium.img

cp bootboot.efi /build/out/bootboot.efi
cp helium /build/out/helium
cp helium.img /build/out/helium.img
cp helium.dbg /build/out/helium.dbg