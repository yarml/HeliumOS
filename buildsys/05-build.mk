
$(BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)

.PHONY: clean clean-cache clean-all

clean:
	cargo clean
	$(RM) $(CLEAN)

clean-cache:
	$(RM) $(CACHE_DIR)

# Will remove EVERYTHING inluding gcc and binutils builds
clean-all: clean gcc-rm binutils-rm mkbootimg-rm bootboot-rm clean-cache

export CC = $(BUILD_CC)
