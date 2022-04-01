# Targets to build kernel elf

SRCS := $(shell $(FIND) kernel/src -name "*.c")
OBJS := $(patsubst %,$(OUT_DIR)/%.o,$(SRCS))
KERNEL_BIN := $(INITRD_SYSROOT)/sys/helium

CFLAGS := -mno-red-zone -Wall -fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib
INC_FLAGS := -I$(KERNEL_INC_DIR) -I$(BOOTBOOT_DIST_DIR)
STRIPFLAGS :=  -s -K mmio -K fb -K bootboot -K environment -K initstack

HELIUM_IMG := $(OUT_DIR)/helium.img


$(OUT_DIR)/%.c.o: %.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) $(INC_FLAGS) -o $@ -c $^

# Will also build the toolchain if not available
$(KERNEL_BIN): $(HOST_CC) $(LINKER_SCRIPT) $(OBJS)
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) $(OBJS) -o $@ -T $(LINKER_SCRIPT)
	$(HOST_STRIP) $(STRIPFLAGS) $@
	$(MKBOOTIMG_BIN) check $@

$(HELIUM_IMG): $(BOOTBOOT_CFG) $(BOOTIMG_CFG) $(KERNEL_BIN) $(INITRD_SYSROOT) $(HOST_SYSROOT) $(MKBOOTIMG_BIN)
	$(MKDIR) -p $(dir $@)
	$(MKBOOTIMG_BIN) $(BOOTIMG_CFG) $@

.PHONY: kernel bootimg
kernel: $(KERNEL_BIN)

bootimg: $(HELIUM_IMG)
