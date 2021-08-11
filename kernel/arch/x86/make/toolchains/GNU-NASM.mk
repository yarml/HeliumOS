AS       ?= nasm -f elf32 -o
CC       ?= i686-elf-gcc -c -o
CXX      ?= i686-elf-g++ -c -o
LD       ?= i686-elf-g++ -o
SL       ?= i686-elf-ar
ASFLAGS  :=
CFLAGS   := -std=gnu17
CXXFLAGS := -std=gnu++20 -fno-exceptions -fno-rtti -fno-use-cxa-atexit
CCFLAGS  := -ffreestanding -Wall -Wextra -O0 -g -masm=intel -fno-builtin
LDFLAGS  := -nostdlib -g -T $(LINK_SCRIPT) -L$(dir $(ARCHOUT)) -l:$(notdir $(ARCHOUT))
SLFLAGS  := rc