# Targets to build kernel elf


KERNEL_BIN := $(INITRD_SYSROOT)sys/helium
KERNEL_SYM := $(INITRD_SYSROOT)sys/ksym
KERNEL_OUT := $(ROOT_DIR)target/helium/debug/helium
STRIPFLAGS :=  -s -K mmio -K fb -K bootboot -K environment -K initstack

CLEAN += $(INITRD_SYSROOT)sys/

KERNEL_SRC = $(shell $(FIND) src/ -name "*.rs")


# Link target
# Will also build the toolchain if not available
$(KERNEL_BIN): $(BINUTILS_DEP) $(MKBOOTIMG_BIN) $(LINKER_SCRIPT) $(KERNEL_SRC)
	$(MKDIR) -p $(@D)
	$(MKDIR) -p $(OUT_DIR)
	cargo xbuild --target $(TRIPLET_CFG)
	$(HOST_OBJCOPY) --only-keep-debug $(KERNEL_OUT) $(OUT_DIR)kernel.dbg
	$(HOST_OBJCOPY) --strip-debug $(KERNEL_OUT)
	$(HOST_STRIP) $(STRIPFLAGS) $(KERNEL_OUT)  -o $@
	$(MKBOOTIMG_BIN) check $@

.PHONY: kernel
kernel: $(KERNEL_BIN)
