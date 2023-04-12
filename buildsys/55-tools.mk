# Compile the quxilary tools used by the buildsystem
# found in $(ROOT_DIR)tools/

MODLDGEN_BIN := $(BUILD_SYSROOT)bin/modldgen

$(MODLDGEN_BIN): $(TOOLS_DIR)modldgen.c
	$(BUILD_CC) $^ -o $@

.PHONY: tool-modldgen
tool-modldgen: $(MODLDGEN_BIN)
