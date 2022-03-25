
$(BUILD_DIR):
	$(MKDIR) -p $(BUILD_DIR)


clean:
	$(RM) -rf $(CLEAN)

# Will remove EVERYTHING inluding gcc and binutils builds
clean-all: clean gcc-rm binutils-rm 
