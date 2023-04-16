FONT_NAME := tamsyn-font-1.11
FONT_LINK := http://www.fial.com/~scott/tamsyn-font/download/$(FONT_NAME).tar.gz
FONT_TYPE := Tamsyn10x20r

FONT_FILE := $(INITRD_SYSROOT)/sys/font.psf

$(FONT_FILE):
	$(CURL) -o $(BUILD_DIR)/font.tar.gz $(FONT_LINK)
	$(CD) $(BUILD_DIR) && $(TAR) -xvf $(BUILD_DIR)/font.tar.gz
	$(CD) $(BUILD_DIR)/$(FONT_NAME)/ && $(GZIP) -d $(FONT_TYPE).psf.gz
	$(MKDIR) -p $(dir $@)
	$(MV) $(BUILD_DIR)/$(FONT_NAME)/$(FONT_TYPE).psf $@

.PHONY: font
font: $(FONT_FILE)
