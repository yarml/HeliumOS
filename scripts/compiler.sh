#!/bin/sh

# Supposed to run from the docker container!
set -e

cd /tmp

mkdir binutils
mkdir binutils-build

tar -xf binutils.tar.gz --strip-components=1 -C binutils

# Even though docker images are append only
# this should help when the image is squashed
rm -rf *.tar.gz

TARGET=x86_64-elf
PREFIX=/toolchain

# Build & Install binutils
cd /tmp/binutils-build
../binutils/configure \
                --target=$TARGET \
                --prefix=$PREFIX \
                --with-sysroot \
                --disable-nls \
                --disable-werror
make -j
make install

cd /
rm -rf /tmp
