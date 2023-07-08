fix-style:
	$(FIND) $(KERNEL_DIR) -iname "*.c" -o -iname "*.h" | $(XARGS) $(CLANG_FORMAT) -i