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

$(MKBOOTIMG_BIN): $(MKBOOTIMG_SRC_DIR)
	$(MAKE) -C $(MKBOOTIMG_SRC_DIR)
	$(RM) -rf $(MKBOOTIMG_SRC_DIR)/../mkbootimg-$(shell uname -s).zip
	$(MV) $(MKBOOTIMG_SRC_DIR)/mkbootimg $(MKBOOTIMG_BIN)
# We touch the file one last time so that the timestamps are guarenteed to be later than the folder's timestamp
	$(TOUCH) $(MKBOOTIMG_BIN)


.PHONY: mkbootimg-src-update mkbootimg mkbootimg-rm mkbootimg-clean
mkbootimg-src-update: $(MKBOOTIMG_SRC_DIR)
mkbootimg: $(MKBOOTIMG_BIN)

mkbootimg-rm:
	$(RM) -rf $(MKBOOTIMG_SRC_DIR)
mkbootimg-clean:
	$(MAKE) -C $(MKBOOTIMG_SRC_DIR) clean


