# Flags for assembler
ASFLAGS  := -f elf32
# Flags for C compiler
CFLAGS   := -std=gnu17
# Flags for C++ compiler
CXXFLAGS := -std=gnu++20 -fno-exceptions -fno-rtti -fno-use-cxa-atexit
# Flags for both C and C++
CCFLAGS  := -ffreestanding -Wall -Wextra -O0 -g -mgeneral-regs-only -masm=intel
# Flags for linker
LDFLAGS  := -ffreestanding -nostdlib -g -T $(LINK_SCRIPT)

# Flags for bochs
BOCHS_FLAGS := -q
# Flags for Qemu
QEMU_FLAGS := -serial stdio

