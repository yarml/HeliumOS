
# Select the toolchain to use
# Each architecture can implement its own set of toolchains
# For example the x86 architecture currently supports the following toolchains
# LLVM-NASM and GNU-NASM
# You can have a look at kernel/arch/demo/make/toolchains/EXAMPLE.mk for
# more details about toolchains
TOOLCHAIN ?= LLVM-NASM

# Basic unix commands
# Make sure that any equivalent of mkdir and rm can work recursicely
MKDIR := mkdir -p
RM    := rm -rf
CP    := cp

# A command to make sure that an iso is multiboot v1 compliant
ISMULTIBOOT := grub-file --is-x86-multiboot

# A command to generate a multiboot compliant iso, make sure the first argument is the output file
MKISO := grub-mkrescue -o

# Bochs and Qemu VMs
BOCHS := bochs
QEMU  := qemu-system-x86_64

# The following line includes the toolchain from the specified architecture
include $(ARCH_MAKE)/toolchains/$(TOOLCHAIN).mk
