AS       := nasm -f elf32 -o
CC       ?= clang -target i686-pc-elf -c -o
CXX      ?= clang -target i686-pc-elf -c -o
LD       ?= clang -target i686-pc-elf -fuse-ld=gold -o
SL       ?= llvm-ar
ASFLAGS  :=
CFLAGS   := -std=gnu17
CXXFLAGS := -std=gnu++20 -fno-exceptions -fno-rtti -fno-use-cxa-atexit
CCFLAGS  := -ffreestanding -Wall -Wextra -O0 -g -masm=intel -fno-builtin
LDFLAGS  := -nostdlib -g -T $(LINK_SCRIPT) -L$(dir $(ARCHOUT)) -l:$(notdir $(ARCHOUT))
SLFLAGS  := rc