
# Flags for linker, compilers and assembler are part of toolchain definitions
# Have a look at kernel/arch/demo/make/toolchains/EXAMPLE.mk for more details

# Flags for bochs
BOCHS_FLAGS := -q
# Flags for Qemu
QEMU_FLAGS := -serial stdio

# M4 Flags
M4FLAGS := -D ARCH=$(ARCH)                  \
 -D ISO=$(ISO)                               \
 -D KERNEL=$(KERNEL)                          \
 -D BOCHS_ROM_IMAGE=$(BOCHS_ROM_IMAGE)         \
 -D BOCHS_VGA_ROM_IMAGE=$(BOCHS_VGA_ROM_IMAGE)  \
 -D GRUB_TIMEOUT=$(GRUB_TIMEOUT)

