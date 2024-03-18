FROM debian:bookworm

ARG CPU MEMORY

RUN apt update &&\
    apt -y upgrade &&\
    apt -y install --no-install-recommends qemu-system ovmf

EXPOSE 5900
COPY --chmod=0755 ./scripts/qemu.sh /vm/qemu.sh
ENTRYPOINT [ "/vm/qemu.sh" ]
