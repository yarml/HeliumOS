AS       := nasm -o
CC       := i686-elf-gcc -c -o
CXX      := i686-elf-g++ -c -o
LD       := i686-elf-g++ -o
ASFLAGS  := -f elf32
CFLAGS   := -std=gnu17
CXXFLAGS := -std=gnu++20 -fno-exceptions -fno-rtti -fno-use-cxa-atexit
CCFLAGS  := -ffreestanding -Wall -Wextra -O0 -g -mgeneral-regs-only
LDFLAGS  := -ffreestanding -nostdlib -g -T kernel/linker.ld
INCLUDES := kernel/include

MKDIR := mkdir -p
RM    := rm -rf
CP    := cp
ISMULTIBOOT := grub-file --is-x86-multiboot
MKISO := grub-mkrescue -o

BOCHS := bochs
BOCHS_FLAGS := -q

BOCHS_CONFIG := bochs.cfg
BOCHSRC := bochsrc

QEMU := qemu-system-x86_64
QEMU_FLAGS := -serial stdio

INCLUDE_FLAGS := $(patsubst %,-I%,$(INCLUDES))

KERNEL := helium.kernel
ISO := helium.iso

SRCD := kernel/src
BDIR := build
OBJD := $(BDIR)/obj
KOUT := $(BDIR)/$(KERNEL)
RDIR := run
ISODIR := isodir
ISORUN := $(RDIR)/$(ISO)
ISOBOOT := $(ISODIR)/boot
BOCHSLOG := $(RDIR)/bochslog.txt
BOCHSCOPY := $(RDIR)/copy.txt
BOCHSCOM := $(RDIR)/com1.out
TOCLEAN := $(RDIR)/bx_enh_dbg.ini $(RDIR)/mem.out

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASEXT  := .asm
CEXT   := .c
CXXEXT := .cpp

ASSOURCES  := $(call rwildcard,$(SRCD),*$(ASEXT))
CSOURCES   := $(call rwildcard,$(SRCD),*$(CEXT))
CXXSOURCES := $(call rwildcard,$(SRCD),*$(CXXEXT))

ASOBJECTS  := $(patsubst %,%.o,$(ASSOURCES))
COBJECTS   := $(patsubst %,%.o,$(CSOURCES))
CXXOBJECTS := $(patsubst %,%.o,$(CXXSOURCES))

BASOBJECTS  := $(patsubst %,$(OBJD)/%,$(ASOBJECTS))
BCOBJECTS   := $(patsubst %,$(OBJD)/%,$(COBJECTS))
BCXXOBJECTS := $(patsubst %,$(OBJD)/%,$(CXXOBJECTS))

.PHONY: all kernel iso bochs qemu qemu-gdb gdb clean

all: iso
	@echo Done

kernel: $(KOUT)
iso: $(ISORUN)

bochs: $(ISORUN)
	@cd run && $(BOCHS) $(BOCHS_FLAGS) -f $(BOCHS_CONFIG) -rc $(BOCHSRC)

qemu: $(ISORUN)
	@cd run && $(QEMU) $(QEMU_FLAGS) -cdrom $(ISO)

qemu-gdb: $(ISORUN)
	@cd run && $(QEMU) $(QEMU_FLAGS) -s -S -cdrom $(ISO)

gdb: $(ISORUN)
	@cd run && gdb

clean:
	@echo Cleaning
	@$(RM) $(TOCLEAN) $(BDIR) $(ISOBOOT)/$(KERNEL) $(BOCHSLOG) $(ISORUN) $(BOCHSCOM) $(BOCHSCOPY)

$(ISORUN): $(KOUT)
	@echo Generating ISO file for the OS
	@$(MKDIR) $(ISOBOOT)
	@$(CP) $(KOUT) $(ISOBOOT)/$(KERNEL)
	@$(MKDIR) $(RDIR)
	@$(MKISO) $(ISORUN) $(ISODIR)

$(KOUT): $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS)
	@echo Linking
	@$(LD) $(KOUT) $(LDFLAGS) $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS)
	@$(ISMULTIBOOT) $(KOUT) || (echo Output file is not multiboot complient; exit 1)

$(OBJD)/%$(CEXT).o: %$(CEXT)
	@echo Compiling C source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(CC) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<

$(OBJD)/%$(CXXEXT).o: %$(CXXEXT)
	@echo Compiling C++ source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(CXX) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<

$(OBJD)/%$(ASEXT).o: %$(ASEXT)
	@echo Assembling: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(AS) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(ASFLAGS) $<
