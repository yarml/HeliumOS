
$(BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)


clean:
	$(RM) $(CLEAN)

# Will remove EVERYTHING inluding gcc and binutils builds
clean-all: clean gcc-rm binutils-rm mkbootimg-rm 

export CC = $(BUILD_CC)
