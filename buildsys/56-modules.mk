
MODULE_LINKSCRIPT := $(BUILD_DIR)module.ld

$(MODULE_LINKSCRIPT): $(KERNEL_BIN) $(MODLDGEN_BIN)
	$(info Remaking $@ because '$?' changed)
	$(MODLDGEN_BIN) $(KERNEL_SYM) $@

.PHONY: modld
modld: $(MODULE_LINKSCRIPT)

MODULES_ALL :=
