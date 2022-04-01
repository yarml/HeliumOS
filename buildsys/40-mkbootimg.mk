# Targets to update mkbootimg source code, and build it

MKBOOTIMG_LINK := https://gitlab.com/bztsrc/bootboot/-/archive/master/bootboot-master.tar.gz?path=mkbootimg

MKBOOTIMG_SRC_DIR   := $(EXT_SRC_DIR)/mkbootimg/

MKBOOTIMG_PREFIX := $(BUILD_SYSROOT)

MKBOOTIMG_BIN := $(MKBOOTIMG_PREFIX)/bin/mkbootimg

$(MKBOOTIMG_SRC_DIR):
	$(MKDIR) -p  $(BUILD_DIR)
	$(MKDIR) -p $(MKBOOTIMG_SRC_DIR)
	$(CURL) -o $(BUILD_DIR)/mkbootimg.tar.gz $(MKBOOTIMG_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)/mkbootimg.tar.gz
	$(MV) $(BUILD_DIR)/bootboot-master-mkbootimg/mkbootimg/* $(MKBOOTIMG_SRC_DIR)
# We modify some files to use our build of bootboot and only include the efi version in the boot image
	$(SED) -i -f $(SEDDIR)/mkbootimg_makefile.sed \
		-e "s|#CC#|${CC}|g" -e "s|#BOOTBOOT_BIN#|$(BOOTBOOT_BIN)|g" \
		$(MKBOOTIMG_SRC_DIR)/Makefile
	$(SED) -i -f $(SEDDIR)/mkbootimg_esp.c.sed $(MKBOOTIMG_SRC_DIR)/esp.c
	$(RM) $(MKBOOTIMG_SRC_DIR)/data.*

$(MKBOOTIMG_BIN): $(MKBOOTIMG_SRC_DIR) $(BOOTBOOT_BIN)
	$(RM) $(MKBOOTIMG_SRC_DIR)/data.*
	$(MAKE) -C $(MKBOOTIMG_SRC_DIR) data.c
	$(MAKE) -C $(MKBOOTIMG_SRC_DIR)
	$(RM) $(MKBOOTIMG_SRC_DIR)/../mkbootimg-$(shell uname -s).zip
	$(MV) $(MKBOOTIMG_SRC_DIR)/mkbootimg $(MKBOOTIMG_BIN)
	$(TOUCH) $(MKBOOTIMG_BIN)


.PHONY: mkbootimg-src-update mkbootimg mkbootimg-rm mkbootimg-clean
mkbootimg-src-update: $(MKBOOTIMG_SRC_DIR)
mkbootimg: $(MKBOOTIMG_BIN)

mkbootimg-rm:
	$(RM) $(MKBOOTIMG_SRC_DIR)
mkbootimg-clean:
	$(MAKE) -C $(MKBOOTIMG_SRC_DIR) clean


