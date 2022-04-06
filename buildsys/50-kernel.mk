# Targets to build kernel elf


SRCS := $(shell $(FIND) $(KERNEL_SRC_DIR) -name "*.c" -o -name "*.asm")
OBJS := $(patsubst %,$(OUT_DIR)/%.o,$(SRCS))
FONT_O := $(OUT_DIR)/font.o
KERNEL_BIN := $(INITRD_SYSROOT)/sys/helium

FONT_NAME := tamsyn-font-1.11
FONT_TYPE := Tamsyn8x15r
FONT_LINK := http://www.fial.com/~scott/tamsyn-font/download/$(FONT_NAME).tar.gz

ASFLAGS := -felf64
CFLAGS := -mno-red-zone -Wall -fpic -ffreestanding -fno-stack-protector -nostdlib
INC_FLAGS := -I$(KERNEL_INC_DIR) -I$(BOOTBOOT_DIST_DIR) -I$(STD_INC)
STRIPFLAGS :=  -s -K mmio -K fb -K bootboot -K environment -K initstack

HELIUM_IMG := $(OUT_DIR)/helium.img

# Compile targets
$(OUT_DIR)/%.c.o: %.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) $(INC_FLAGS) -o $@ -c $^

$(OUT_DIR)/%.asm.o: %.asm
	$(MKDIR) -p $(dir $@)
	$(HOST_AS) $(ASFLAGS) -o $@ $^

# Link target
# Will also build the toolchain if not available
$(KERNEL_BIN): $(FONT_O) $(HOST_CC) $(LINKER_SCRIPT) $(OBJS)
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) $(OBJS) $(FONT_O) -o $@ -T $(LINKER_SCRIPT)
	$(HOST_STRIP) $(STRIPFLAGS) $@
	$(MKBOOTIMG_BIN) check $@

$(HELIUM_IMG): $(BOOTBOOT_CFG) $(BOOTIMG_CFG) $(KERNEL_BIN) $(INITRD_SYSROOT) $(HOST_SYSROOT) $(MKBOOTIMG_BIN)
	$(MKDIR) -p $(dir $@)
	$(MKBOOTIMG_BIN) $(BOOTIMG_CFG) $@

# Font file
$(FONT_FILE):
	$(CURL) -o $(BUILD_DIR)/font.tar.gz $(FONT_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)/font.tar.gz
	$(CD) $(BUILD_DIR)/$(FONT_NAME)/ && $(GZIP) -d $(FONT_TYPE).psf.gz
	$(MV) $(BUILD_DIR)/$(FONT_NAME)/$(FONT_TYPE).psf $(KERNEL_DIR)/font.psf

$(FONT_O): $(FONT_FILE) $(BINUTILS_DEP)
	$(MKDIR) -p $(dir $@)
	$(HOST_LD) -r -b binary -o $@ $(FONT_FILE)


.PHONY: font kernel bootimg
font: $(FONT_FILE)
kernel: $(KERNEL_BIN)
bootimg: $(HELIUM_IMG)
