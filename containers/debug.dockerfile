FROM rust:1.77.2-bookworm

COPY ./rust-toolchain /build/env/rust-toolchain
COPY --chmod=0755 ./config/env/common.sh /build/env/common.sh

RUN . /build/env/common.sh &&\
    apt-get update &&\
    apt-get -y upgrade &&\
    apt-get -y install --no-install-recommends qemu-system ovmf gdb tmux socat &&\
    rustup toolchain install ${RUST_TOOLCHAIN} &&\
    rustup component add rust-src --toolchain ${RUST_TOOLCHAIN}


COPY --chmod=0755 ./scripts/debug.sh /vm/debug.sh
ENTRYPOINT [ "/vm/debug.sh" ]
