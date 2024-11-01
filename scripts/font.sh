#!/bin/sh

# Supposed to run from the docker container!

set -e

FONT_TYPE=Tamsyn8x16r

mkdir font
mkdir -p /build/intermediate

tar -xf font.tar.gz --strip-components=1 -C font
gzip -d font/${FONT_TYPE}.psf.gz
mv font/${FONT_TYPE}.psf /build/intermediate/font.psf

