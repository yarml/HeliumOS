FROM rust:1.76.0-bookworm

ARG CPU MEMORY

RUN apt update &&\
    apt -y upgrade &&\
    apt -y install --no-install-recommends qemu-system ovmf tmux gdb socat

EXPOSE 5900
COPY --chmod=0755 ./scripts/debug.sh /vm/debug.sh
ENTRYPOINT [ "/vm/debug.sh" ]
