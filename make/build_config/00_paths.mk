# Architecture output
ARCH          ?= i686
# Output files names
KERNEL        := helium-$(ARCH).kernel
ISO           := helium-$(ARCH).iso
ARCH_FILE     := arch-$(ARCH).a

# File names
KERNEL_DIR    := kernel
CORE_DIR      := core
ARCH_DIR      := arch
TEMPLATES_DIR := templates
# Build directory
BDIR          ?= build
# Run directory
RDIR          ?= run
ISODIR        := isodir
INCLUDE_DIR   := include
CONFIG_DIR    := config
SRC_DIR       := src
LINK_SCRIPT_N := linker.ld

# You shouldn't really need to change those, but you can
# Changing these may break something, if it is the case report it as a bug
# as these variables are supposed to be configurable
KERNEL_CORE   := $(KERNEL_DIR)/$(CORE_DIR)
KERNEL_ARCH   := $(KERNEL_DIR)/$(ARCH_DIR)
CURRENT_ARCH  := $(KERNEL_ARCH)/$(ARCH)
ARCH_MAKE     := $(CURRENT_ARCH)/make
ARCH_SRC      := $(CURRENT_ARCH)/$(SRC_DIR)
ARCH_INCLUDES := $(CURRENT_ARCH)/$(INCLUDE_DIR) $(CURRENT_ARCH)/$(CONFIG_DIR)
LINK_SCRIPT   := $(CURRENT_ARCH)/$(LINK_SCRIPT_N)
SRCD          := $(KERNEL_CORE)/$(SRC_DIR)
INCLUDES      := $(KERNEL_CORE)/$(INCLUDE_DIR) $(KERNEL_CORE)/$(CONFIG_DIR)
OBJD          := $(BDIR)/obj
KOUT          := $(BDIR)/$(KERNEL)
ARCHOUT       := $(BDIR)/$(ARCH_FILE)
ISORUN        := $(RDIR)/$(ISO)
ISOBOOT       := $(ISODIR)/boot
ISOGRUB       := $(ISOBOOT)/grub
GRUBCFG       := $(ISOGRUB)/grub.cfg

# Bochs files
BOCHSLOG      := $(RDIR)/bochslog.txt
BOCHSCOPY     := $(RDIR)/copy.txt
BOCHSCOM      := $(RDIR)/com1.out
# The following are relative to $(RDIR)
BOCHS_CONFIG  := bochs.cfg
BOCHSRC       := bochsrc

# Additional files to clean with `make clean`
TOCLEAN       := $(RDIR)/bx_enh_dbg.ini $(RDIR)/mem.out
