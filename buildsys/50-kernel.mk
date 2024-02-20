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
	cp $(KERNEL_OUT) $@
	$(MKBOOTIMG_BIN) check $@

.PHONY: kernel
kernel: $(KERNEL_BIN)
