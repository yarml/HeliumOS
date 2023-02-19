# Targets to update binutils source code, configure binutils, and build binutils

BINUTILS_VERSION := 2.39
BINUTILS_NAME := binutils-$(BINUTILS_VERSION)
BINUTILS_LINK := https://ftp.gnu.org/gnu/binutils/$(BINUTILS_NAME).tar.gz

BINUTILS_SRC_DIR := $(EXT_SRC_DIR)binutils/
BINUTILS_BUILD_DIR := $(EXT_BUILD_DIR)binutils/

BINUTILS_TARGET 		 := x86_64-elf
BINUTILS_PREFIX 		 := $(BUILD_SYSROOT)
BINUTILS_CONFIGURE_FLAGS := --with-sysroot --disable-nls --disable-werror

BINUTILS_MAKEFILE := $(BINUTILS_BUILD_DIR)Makefile
BINUTILS_TOOLCHAIN_PREFIX := $(BINUTILS_PREFIX)bin/$(BINUTILS_TARGET)
BINUTILS_LD := $(BINUTILS_TOOLCHAIN_PREFIX)-ld
BINUTILS_DEP := $(BINUTILS_LD)

$(BINUTILS_SRC_DIR):
	$(MKDIR) -p $(BUILD_DIR)
	$(MKDIR) -p $(BINUTILS_SRC_DIR)
	$(CURL) -o $(BUILD_DIR)binutils.tar.gz $(BINUTILS_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)binutils.tar.gz
	$(MV) $(BUILD_DIR)$(BINUTILS_NAME)/* $(BINUTILS_SRC_DIR)

$(BINUTILS_MAKEFILE): $(BINUTILS_SRC_DIR)
	$(MKDIR) -p $(BINUTILS_BUILD_DIR)
	$(CD) $(BINUTILS_BUILD_DIR) && $(BINUTILS_SRC_DIR)configure --target=$(BINUTILS_TARGET) \
		--prefix=$(BINUTILS_PREFIX) $(BINUTILS_CONFIGURE_FLAGS)

$(BINUTILS_LD): $(BINUTILS_MAKEFILE)
	$(MAKE) -C $(BINUTILS_BUILD_DIR)
	$(MAKE) -C $(BINUTILS_BUILD_DIR) install

.PHONY: binutils-src-update binutils-configure binutils binutils-rm binutils-clean
binutils-src-update: $(BINUTILS_SRC_DIR)
binutils-configure:  $(BINUTILS_MAKEFILE)
binutils: $(BINUTILS_LD)

binutils-rm: binutils-clean
	$(RM) $(BINUTILS_SRC_DIR)
binutils-clean:
	$(RM)  $(BINUTILS_BUILD_DIR)

