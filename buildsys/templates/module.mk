# Template file: module.mk
# <mod-name>: module name

MODULE_<mod-name>_SRC := $(shell $(FIND) $(MODULES_DIR)<mod-name>/ -name "*.c" -o -name "*.asm")
MODULE_<mod-name>_OBJ := $(patsubst $(MODULES_DIR)%,$(OUT_DIR)modules/%.o,$(MODULE_<mod-name>_SRC))

MODULE_<mod-name>_CO := $(OUT_DIR)modules/<mod-name>/<mod-name>.co
MODULE_<mod-name>_BIN := $(INITRD_SYSROOT)modules/<mod-name>.mod

MODULE_<mod-name>_ASFLAGS := -felf64
MODULE_<mod-name>_CFLAGS := -mno-red-zone -Wall -ffreestanding -fno-stack-protector \
		  -nostdlib -Werror -fno-asynchronous-unwind-tables -fpic

ifeq ($(M),DEBUG)
MODULE_<mod-name>_CFLAGS += -DHELIUM_DEBUG -O0 -ggdb3
else
MODULE_<mod-name>_CFLAGS += -O3
endif

$(MODULE_<mod-name>_BIN): $(MODLD_BIN) $(MODULE_<mod-name>_OBJ)
	$(MKDIR) -p $(dir $@)
	$(MKDIR) -p $(dir $(MODULE_<mod-name>_CO))
	$(HOST_LD) -r $(MODULE_<mod-name>_OBJ) -o $(MODULE_<mod-name>_CO)
	$(HOST_OBJCOPY) --only-keep-debug $(MODULE_<mod-name>_CO) $(OUT_DIR)modules/<mod-name>/<mod-name>.dbg
	$(HOST_OBJCOPY) --strip-debug $(MODULE_<mod-name>_CO)
	$(HOST_OBJCOPY) --strip-unneeded $(MODULE_<mod-name>_CO)
	$(MODLD_BIN) $(MODULE_<mod-name>_CO) $@

$(OUT_DIR)modules/<mod-name>/%.c.o: $(MODULES_DIR)<mod-name>/%.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) $(MODULE_<mod-name>_CFLAGS) $(INC_FLAGS) -o $@ -c $^

$(OUT_DIR)modules/<mod-name>/%.int.c.o: $(MODULES_DIR)<mod-name>/%.int.c
	$(MKDIR) -p $(dir $@)
	$(HOST_CC) -mgeneral-regs-only $(MODULE_<mod-name>_CFLAGS) $(INC_FLAGS) -o $@ -c $^

$(OUT_DIR)modules/<mod-name>/%.asm.o: $(MODULES_DIR)<mod-name>/%.asm
	$(MKDIR) -p $(dir $@)
	$(HOST_AS) $(MODULE_<mod-name>_ASFLAGS) $(INC_FLAGS) -o $@ $^

MODULES_ALL += $(MODULE_<mod-name>_BIN)

.PHONY: module-<mod-name> module-<mod-name>-clean
module-<mod-name>: $(MODULE_<mod-name>_BIN)

module-<mod-name>-clean:
	$(RM) $(MODULE_<mod-name>_BIN) $(MODULE_<mod-name>_OBJ)
