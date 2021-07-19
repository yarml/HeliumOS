# Output files names
KERNEL := helium.kernel
ISO := helium.iso
# Directories names
KERNEL_DIR  := kernel
BDIR        := build
RDIR        := run
ISODIR      := isodir

# You shouldn't really need to change those
SRCD        := $(KERNEL_DIR)/src
LINK_SCRIPT := $(KERNEL_DIR)/linker.ld
INCLUDES    := $(KERNEL_DIR)/include $(KERNEL_DIR)/config
OBJD        := $(BDIR)/obj
KOUT        := $(BDIR)/$(KERNEL)
ISORUN      := $(RDIR)/$(ISO)
ISOBOOT     := $(ISODIR)/boot
ISOGRUB     := $(ISOBOOT)/grub
GRUBCFG     := $(ISOGRUB)/grub.cfg

# Bochs files
BOCHSLOG    := $(RDIR)/bochslog.txt
BOCHSCOPY   := $(RDIR)/copy.txt
BOCHSCOM    := $(RDIR)/com1.out
# The following are relative to $(RDIR)
BOCHS_CONFIG := bochs.cfg
BOCHSRC := bochsrc

# Additional files to clean with `make clean`
TOCLEAN     := $(RDIR)/bx_enh_dbg.ini $(RDIR)/mem.out
