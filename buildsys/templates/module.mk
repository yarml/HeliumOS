# Template file: module.mk
# <mod-name>: module name

MODULE_<mod-name>_SRC := $(shell $(FIND) $(MODULES_DIR)<mod-name>/ -name "*.c" -o -name "*.asm")
MODULE_<mod-name>_OBJ := $(patsubst $(MODULES_DIR)%,$(OUT_DIR)modules/%.o,$(MODULE_<mod-name>_SRC))

MODULE_<mod-name>_BIN := $(INITRD_SYSROOT)modules/<mod-name>.mod

$(MODULE_<mod-name>_BIN): $(MODULE_LINKSCRIPT) $(MODULE_<mod-name>_OBJ)
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS) $(MODULE_<mod-name>_OBJ) -o $@ -T $(MODULE_LINKSCRIPT)

$(OUT_DIR)modules/<mod-name>/%.c.o: $(MODULES_DIR)<mod-name>/%.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(CFLAGS)  $(INC_FLAGS) -o $@ -c $^

MODULES_ALL += $(MODULE_<mod-name>_BIN)

.PHONY: module-<mod-name>
module-<mod-name>: $(MODULE_<mod-name>_BIN)
