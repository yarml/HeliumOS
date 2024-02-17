LIB_DIR_<name> := $(UPDIR)<path>/
LIB_SRCDIR_<name> := $(LIB_DIR_<name>)src/
LIB_INCDIR_<name> := $(LIB_DIR_<name>)include/
LIB_OUTDIR_<name> := $(UPOUT_DIR)<name>/
LIB_OBJDIR_<name> := $(LIB_OUTDIR_<name>)obj/
LIB_TARGET_<name> := $(LIB_OUTDIR_<name>)lib<name>.a

LIB_SRC_<name> := $(shell $(FIND) $(LIB_SRCDIR_<name>) -name "*.c" -o -name "*.asm")
LIB_OBJ_<name> := $(patsubst $(LIB_SRCDIR_<name>)%,$(LIB_OBJDIR_<name>)%.o,$(LIB_SRC_<name>))

LIB_INCLUDE_FLAGS_<name> := -I$(LIB_INCDIR_<name>)

UPLIB += $(LIB_TARGET_<name>)

$(LIB_OBJDIR_<name>)%.c.o: $(LIB_SRCDIR_<name>)%.c
	$(MKDIR) -p $(@D)
	$(HOST_CC) -Werror -ffreestanding -ggdb3 -c $^ -o $@ $(LIB_INCLUDE_FLAGS_<name>)

$(LIB_OBJDIR_<name>)%.asm.o: $(LIB_SRCDIR_<name>)%.asm
	$(MKDIR) -p $(@D)
	$(HOST_AS) -felf64 $^ -o $@ $(LIB_INCLUDE_FLAGS_<name>)

$(LIB_TARGET_<name>): $(LIB_OBJ_<name>)
	$(MKDIR) -p $(@D)
	$(RM) $@
	$(HOST_AR) rcs $@ $^

$(INITRD_SYSROOT)lib/lib<name>.a: $(LIB_TARGET_<name>)
	$(MKDIR) -p $(@D)
	$(CP) $^ $@

.PHONY: up-lib<name>
up-lib<name>: $(LIB_TARGET_<name>)