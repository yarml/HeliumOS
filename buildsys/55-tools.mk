# Compile the quxilary tools used by the buildsystem
# found in $(ROOT_DIR)tools/

MODLD_BIN := $(BUILD_SYSROOT)bin/modld

$(MODLD_BIN): $(wildcard $(TOOLS_DIR)modld/*.c)
	$(BUILD_CC) -Wall -Werror $^ -o $@ -O0 -ggdb3

CLEAN += $(MODLD_BIN)

.PHONY: tool-modld
tool-modld: $(MODLD_BIN)
