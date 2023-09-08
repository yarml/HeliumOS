
.PHONY: dep
dep:
	$(info ***README*** It is preferred to refer to docs/Dependencies.md and install dependencies on your own, or check $(BUILDSYS)90-dep.mk)
	$(info ***README*** This will install all required and optional dependencies)
	$(info ***README*** If this fails, you should probably run make dep as root (sudo make dep))

	@$(ECHO) -n 'Are you sure you want to continue? [Y/n]: ' && $(READ) ans && \
		if [ $${ans:-'N'} != 'y' ] && [ $${ans:-'N'} != 'Y' ]; then \
			$(ECHO) Operation canceled; \
			$(EXIT) 1; \
		fi

	$(info Installing dependencies for '$(DISTRO)')
ifeq ($(DISTRO),Arch)
	pacman -S base-devel gmp libmpc mpfr nasm zip gnu-efi qemu edk2-ovmf gdb tmux socat python clang
else ifeq ($(DISTRO),ManjaroLinux)
	pamac install base-devel gmp libmpc mpfr nasm zip gnu-efi qemu edk2-ovmf gdb tmux socat python clang
else
	$(error Unsupported distro $(DISTRO), you'll need to install dependencies on your own)
endif
