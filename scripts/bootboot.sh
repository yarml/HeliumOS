#!/bin/sh

# Supposed to run from the docker container!

set -e

mkdir bootboot
mkdir -p /build/intermediate

tar -xf bootboot.tar.gz --strip-components=2 -C bootboot
tar -xf mkbootimg.tar.gz --strip-components=1

cd /tmp/mkbootimg
make -j

mv mkbootimg /toolchain/bin/mkbootimg

mv /tmp/bootboot/bootboot.efi /build/intermediate

cd /build
rm -rf toolchain