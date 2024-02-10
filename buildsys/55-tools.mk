UPCONFIG_BIN := $(BUILD_SYSROOT)bin/upconfig

UPCONFIG_DIR := $(TOOLS_DIR)upconfig/

$(UPCONFIG_BIN): $(UPCONFIG_DIR)/Cargo.toml $(wildcard $(UPCONFIG_DIR)/src/*.rs)
	$(CD) $(UPCONFIG_DIR) && $(BUILD_CARGO) build --release
	$(CP) $(UPCONFIG_DIR)target/release/upconfig $(UPCONFIG_BIN)

CLEAN += $(UPCONFIG_BIN)

.PHONY: tool-upconfig
tool-upconfig: $(UPCONFIG_BIN)