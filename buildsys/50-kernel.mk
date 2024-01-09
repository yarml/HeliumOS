# Targets to build kernel elf


SRCS := $(shell $(FIND) $(KERNEL_SRC_DIR) -name "*.c" -o -name "*.asm")
OBJS := $(patsubst $(KERNEL_SRC_DIR)%,$(OUT_DIR)kernel/%.o,$(SRCS))
KERNEL_BIN := $(INITRD_SYSROOT)sys/helium

KERNEL_SYM := $(INITRD_SYSROOT)sys/ksym

ASFLAGS := -felf64
CFLAGS := -mno-red-zone -Wall -fpic -ffreestanding \
		  -nostdlib -Werror -fno-asynchronous-unwind-tables
INC_FLAGS := -I$(KERNEL_INC_DIR) -I$(BOOTBOOT_DIST_DIR) -I$(STD_INC)
STRIPFLAGS :=  -s -K mmio -K fb -K bootboot -K environment -K initstack

ifeq ($(M),DEBUG)
CFLAGS += -DHELIUM_DEBUG -O0 -ggdb3 -fstack-protector-all
else
CFLAGS += -O3
endif

CLEAN += $(INITRD_SYSROOT)sys/

# Compile targets
$(OUT_DIR)kernel/%.c.o: $(KERNEL_SRC_DIR)%.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS)  $(INC_FLAGS) -o $@ -c $^

# Some special files need special flags, we handle them separatly
$(OUT_DIR)kernel/%.int.c.o: $(KERNEL_SRC_DIR)%.int.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) -mgeneral-regs-only $(INC_FLAGS) -o $@ -c $^

$(OUT_DIR)kernel/%.asm.o: $(KERNEL_SRC_DIR)%.asm
	$(MKDIR) -p $(dir $@)
	$(HOST_AS) $(ASFLAGS) $(INC_FLAGS) -o $@ $^

# Link target
# Will also build the toolchain if not available
$(KERNEL_BIN): $(HOST_CC) $(MKBOOTIMG_BIN) $(LINKER_SCRIPT) $(OBJS)
	$(MKDIR) -p $(dir $@)
	$(MKDIR) -p $(OUT_DIR)
	$(HOST_CC) $(CFLAGS) $(OBJS) -o $(OUT_DIR)kernel.elf -T $(LINKER_SCRIPT)
	$(HOST_OBJCOPY) --only-keep-debug $(OUT_DIR)kernel.elf $(OUT_DIR)kernel.dbg
	$(HOST_OBJCOPY) --strip-debug $(OUT_DIR)kernel.elf
	$(HOST_OBJCOPY) --extract-symbol $(OUT_DIR)kernel.elf $(KERNEL_SYM)
	$(HOST_STRIP) $(STRIPFLAGS) $(OUT_DIR)kernel.elf  -o $@
	$(MKBOOTIMG_BIN) check $@

.PHONY: kernel
kernel: $(KERNEL_BIN)
