
.PHONY: dep
dep:
	$(info Installing dependencies for '$(DISTRO)')
	$(info If this fails, you should probably run make dep as root (sudo make dep))
ifeq ($(DISTRO),Arch)
	pacman -S base-devel gmp libmpc mpfr zip gnu-efi qemu edk2-ovmf gdb tmux socat
else
	$(error Unsupported distro $(DISTRO), you'll need to install dependencies on your own)
endif
