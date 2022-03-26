# Targets to update bootboot source code, and build it
# God do I hate the way bootboot sources are packaged

BOOTBOOT_LINK := https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=x86_64-efi
BOOTBOOT_DIST_LINK := https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=dist

BOOTBOOT_SRC_DIR   := $(EXT_SRC_DIR)/bootboot/
BOOTBOOT_DIST_DIR := $(BOOTBOOT_SRC_DIR)/../dist

BOOTBOOT_PREFIX := $(BUILD_SYSROOT)

BOOTBOOT_BIN := $(BOOTBOOT_DIST_DIR)/bootboot.efi

$(BOOTBOOT_SRC_DIR):
	$(MKDIR) -p $(BUILD_DIR)
	$(MKDIR) -p $(BOOTBOOT_SRC_DIR)
	$(MKDIR) -p $(BOOTBOOT_DIST_DIR)
	$(CURL) -o $(BUILD_DIR)/bootboot.tar.gz $(BOOTBOOT_LINK)
	$(CURL) -o $(BUILD_DIR)/bootboot-dist.tar.gz $(BOOTBOOT_DIST_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)/bootboot.tar.gz && $(TAR) -xvf $(BUILD_DIR)/bootboot-dist.tar.gz
	$(MV) $(BUILD_DIR)/bootboot-master-x86_64-efi/x86_64-efi/* $(BOOTBOOT_SRC_DIR)
	$(MV) $(BUILD_DIR)/bootboot-master-dist/dist/bootboot.h $(BOOTBOOT_DIST_DIR)

$(BOOTBOOT_BIN): $(BOOTBOOT_SRC_DIR)
	$(MAKE) -C $(BOOTBOOT_SRC_DIR)
# We touch the file one last time to make sure the timestamps are later tham the folder's timestamps
	$(TOUCH) $(BOOTBOOT_BIN)
# Move dist/bootboot.efi to where the rest of the build system expects it to be;
# will do when that part of the buildsys is written

.PHONY: bootboot-src-update bootboot bootboot-rm bootboot-clean
bootboot-src-update: $(BOOTBOOT_SRC_DIR)
bootboot: $(BOOTBOOT_BIN)

bootboot-rm:
	$(RM) -rf $(BOOTBOOT_SRC_DIR)
	$(RM) -rf $(BOOTBOOT_DIST_DIR)
bootboot-clean:
	$(MAKE) -C $(BOOTBOOT_SRC_DIR) clean
