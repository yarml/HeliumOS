.PHONY: build run build-run install reboot

build:
	docker compose run --remove-orphans --build build

run:
	docker compose run --remove-orphans --build debug

build-run: build run

# Run as root la7fdek
EFI_ROOT := /boot/efi/

install:
	mkdir -p out/mnt
	mount -o loop,offset=65536 out/helium.iso out/mnt
	cp -r out/mnt/BOOTBOOT $(EFI_ROOT)
	cp out/mnt/BOOTBOOT.BIN $(EFI_ROOT)
	cp out/mnt/EFI/BOOT/BOOTX64.EFI $(EFI_ROOT)HELIUM.EFI
	umount out/mnt
	rm -rf out/mnt
	sync

# Run as root
install-reboot: install
	reboot
