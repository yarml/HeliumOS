FROM rust:1.76.0-bookworm

ARG CPU MEMORY

RUN apt-get update &&\
    apt-get -y upgrade &&\
    apt-get -y install --no-install-recommends qemu-system ovmf gdb

WORKDIR /vm/
ENTRYPOINT [ "rust-gdb" ]
