# Targets to build kernel elf


SRCS := $(shell $(FIND) $(KERNEL_SRC_DIR) -name "*.c" -o -name "*.asm")
OBJS := $(patsubst %,$(OUT_DIR)%.o,$(SRCS))
KERNEL_BIN := $(INITRD_SYSROOT)sys/helium

KERNEL_SYM := $(OUT_DIR)kernel.sym

ASFLAGS := -felf64
CFLAGS := -mno-red-zone -Wall -fpic -ffreestanding -fno-stack-protector \
		  -nostdlib -Werror
INC_FLAGS := -I$(KERNEL_INC_DIR) -I$(BOOTBOOT_DIST_DIR) -I$(STD_INC)
STRIPFLAGS :=  -s -K mmio -K fb -K bootboot -K environment -K initstack

HELIUM_IMG := $(OUT_DIR)helium.img

ifeq ($(M),DEBUG)
CFLAGS += -DHELIUM_DEBUG -O0 -ggdb3
else
CFLAGS += -O3
endif

# Compile targets
$(OUT_DIR)%.c.o: %.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS)  $(INC_FLAGS) -o $@ -c $^

# Some special files need special flags, we handle them separatly
$(OUT_DIR)%.int.c.o: %.int.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) -mgeneral-regs-only $(INC_FLAGS) -o $@ -c $^

$(OUT_DIR)%.asm.o: %.asm
	$(MKDIR) -p $(dir $@)
	$(HOST_AS) $(ASFLAGS) $(INC_FLAGS) -o $@ $^

# Link target
# Will also build the toolchain if not available
$(KERNEL_BIN): $(HOST_CC) $(MKBOOTIMG_BIN) $(LINKER_SCRIPT) $(OBJS)
	$(MKDIR) -p $(dir $@)
	$(MKDIR) -p $(OUT_DIR)
	$(HOST_CC) $(CFLAGS) $(OBJS) -o $(OUT_DIR)kernel.elf -T $(LINKER_SCRIPT)
	ls -lash build/out
	$(HOST_OBJCOPY) --only-keep-debug $(OUT_DIR)kernel.elf $(OUT_DIR)kernel.dbg
	$(HOST_OBJCOPY) --strip-debug $(OUT_DIR)kernel.elf
	$(HOST_OBJCOPY) --extract-symbol $(OUT_DIR)kernel.elf $(KERNEL_SYM)
	$(HOST_STRIP) $(STRIPFLAGS) $(OUT_DIR)kernel.elf  -o $@
	$(MKBOOTIMG_BIN) check $@

$(HELIUM_IMG): $(BOOTBOOT_CFG) $(BOOTIMG_CFG) $(KERNEL_BIN) $(INITRD_SYSROOT) $(HOST_SYSROOT) $(MKBOOTIMG_BIN)
	$(MKDIR) -p $(dir $@)
	$(MKBOOTIMG_BIN) $(BOOTIMG_CFG) $@

.PHONY: kernel bootimg
kernel: $(KERNEL_BIN)
bootimg: $(HELIUM_IMG)
