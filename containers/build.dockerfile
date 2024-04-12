
FROM rust:1.76.0-bookworm


RUN apt-get update &&\
    apt-get -y upgrade &&\
    apt-get -y install --no-install-recommends curl \
                    build-essential bison flex \
                    libgmp3-dev libmpc-dev libmpfr-dev \
                    texinfo libisl-dev zip nasm &&\
    rustup toolchain install nightly-x86_64-unknown-linux-gnu &&\
    rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu

WORKDIR /tmp

# Build binutils
ENV BINUTILS_VERSION=2.42 \
    PATH="${PATH}:/toolchain/bin"
ENV BINUTILS_URL=https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz
ADD ${BINUTILS_URL} binutils.tar.gz
COPY --chmod=0755 ./scripts/compiler.sh /build/scripts/compiler.sh
RUN /build/scripts/compiler.sh

# Build mkbootimg & download bootboot
ENV BOOTBOOT_URL=https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=dist\
    MKBOOTIMG_URL=https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=mkbootimg
ADD ${BOOTBOOT_URL} bootboot.tar.gz
ADD ${MKBOOTIMG_URL} mkbootimg.tar.gz
COPY --chmod=0755 ./scripts/bootboot.sh /build/scripts/bootboot.sh
RUN /build/scripts/bootboot.sh

WORKDIR /build
COPY --chmod=0755 ./scripts/build.sh /build/scripts/build.sh
ENTRYPOINT [ "/build/scripts/build.sh" ]
