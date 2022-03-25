# Targets to update binutils source code, configure binutils, and build binutils

BINUTILS_VERSION := 2.38
BINUTILS_NAME := binutils-$(BINUTILS_VERSION)
BINUTILS_LINK := https://ftp.gnu.org/gnu/binutils/$(BINUTILS_NAME).tar.gz

BINUTILS_SRC_DIR := $(EXT_SRC_DIR)/binutils
BINUTILS_BUILD_DIR := $(EXT_BUILD_DIR)/binutils

BINUTILS_TARGET 		 := x86_64-elf
BINUTILS_PREFIX 		 := $(BUILD_SYSROOT)
export PATH := ${PATH}:$(BINUTILS_PREFIX)
BINUTILS_CONFIGURE_FLAGS := --with-sysroot --disable-nls --disable-werror

BINUTILS_MAKEFILE := $(BINUTILS_BUILD_DIR)/Makefile
BINUTILS_LD := $(BINUTILS_PREFIX)/bin/$(BINUTILS_TARGET)-ld

$(BINUTILS_SRC_DIR): $(BUILD_DIR)
	$(MKDIR) -p $(BINUTILS_SRC_DIR)
	$(CURL) -o $(BUILD_DIR)/binutils.tar.gz $(BINUTILS_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)/binutils.tar.gz
	$(MV) $(BUILD_DIR)/$(BINUTILS_NAME)/* $(BINUTILS_SRC_DIR)

$(BINUTILS_MAKEFILE): $(BINUTILS_SRC_DIR)
	$(MKDIR) -p $(BINUTILS_BUILD_DIR)
	$(CD) $(BINUTILS_BUILD_DIR) && $(BINUTILS_SRC_DIR)/configure --target=$(BINUTILS_TARGET) \
		--prefix=$(BINUTILS_PREFIX) $(BINUTILS_CONFIGURE_FLAGS)

$(BINUTILS_LD): $(BINUTILS_MAKEFILE)
	$(CD) $(BINUTILS_BUILD_DIR) && $(MAKE)
	$(CD) $(BINUTILS_BUILD_DIR) && $(MAKE) install

.PHONY: binutils-src-update binutils-configure binutils binutils-rm binutils-clean
binutils-src-update: $(BINUTILS_SRC_DIR)
binutils-configure:  $(BINUTILS_MAKEFILE)
binutils: $(BINUTILS_LD)

binutils-rm: binutils-clean
	$(RM) -rf $(BINUTILS_SRC_DIR)
binutils-clean:
	$(RM) -rf  $(BINUTILS_BUILD_DIR)

