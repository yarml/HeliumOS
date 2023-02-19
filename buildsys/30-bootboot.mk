# Targets to update bootboot source code, and build it
# God do I hate the way bootboot sources are packaged

BOOTBOOT_LINK := https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=x86_64-efi
BOOTBOOT_DIST_LINK := https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=dist

BOOTBOOT_SRC_DIR   := $(EXT_SRC_DIR)bootboot/
BOOTBOOT_DIST_DIR := $(BOOTBOOT_SRC_DIR)../dist/

BOOTBOOT_PREFIX := $(BUILD_SYSROOT)

BOOTBOOT_BIN := $(OUT_DIR)bootboot.efi

$(BOOTBOOT_SRC_DIR):
	$(MKDIR) -p $(BUILD_DIR)
	$(MKDIR) -p $(BOOTBOOT_SRC_DIR)
	$(MKDIR) -p $(BOOTBOOT_DIST_DIR)
	$(CURL) -o $(BUILD_DIR)bootboot.tar.gz $(BOOTBOOT_LINK)
	$(CURL) -o $(BUILD_DIR)bootboot-dist.tar.gz $(BOOTBOOT_DIST_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)bootboot.tar.gz && $(TAR) -xvf $(BUILD_DIR)bootboot-dist.tar.gz
	$(MV) $(BUILD_DIR)bootboot-master-x86_64-efi/x86_64-efi/* $(BOOTBOOT_SRC_DIR)
	$(MV) $(BUILD_DIR)bootboot-master-dist/dist/bootboot.h $(BOOTBOOT_DIST_DIR)

$(BOOTBOOT_BIN): $(BOOTBOOT_SRC_DIR)
	$(MAKE) -C $(BOOTBOOT_SRC_DIR)
	$(MKDIR) -p $(dir $(BOOTBOOT_BIN))
	$(MV) $(BOOTBOOT_DIST_DIR)bootboot.efi $(BOOTBOOT_BIN)
	$(TOUCH) $(BOOTBOOT_BIN)

.PHONY: bootboot-src-update bootboot bootboot-rm bootboot-clean
bootboot-src-update: $(BOOTBOOT_SRC_DIR)
bootboot: $(BOOTBOOT_BIN)

bootboot-rm:
	$(RM) $(BOOTBOOT_SRC_DIR)
	$(RM) $(BOOTBOOT_DIST_DIR)
bootboot-clean:
	$(MAKE) -C $(BOOTBOOT_SRC_DIR) clean
