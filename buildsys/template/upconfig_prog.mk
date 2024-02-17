PROG_DIR_<name> := $(UPDIR)<path>/
PROG_SRCDIR_<name> := $(PROG_DIR_<name>)src/
PROG_INCDIR_<name> := $(PROG_DIR_<name>)include/
PROG_OUTDIR_<name> := $(UPOUT_DIR)<path>/
PROG_OBJDIR_<name> := $(PROG_OUTDIR_<name>)obj/
PROG_TARGET_<name> := $(PROG_OUTDIR_<name>)<name>

PROG_SRC_<name> := $(shell $(FIND) $(PROG_SRCDIR_<name>) -name "*.c" -o -name "*.asm")
PROG_OBJ_<name> := $(patsubst $(PROG_SRCDIR_<name>)%,$(PROG_OBJDIR_<name>)%.o,$(PROG_SRC_<name>))

PROG_DEPNAME_<name> := <depname>
PROG_DEPPATH_<name> := <deppath>

PROG_DEPPREF_<name> := $(patsubst %,$(UPOUT_DIR)%/,$(PROG_DEPNAME_<name>))
PROG_DEPNAME_<name> := $(patsubst %,lib%.a,$(PROG_DEPNAME_<name>))
PROG_DEP_<name> := $(join $(PROG_DEPPREF_<name>),$(PROG_DEPNAME_<name>))

PROG_INCLUDE_FLAGS_<name> := -I$(PROG_INCDIR_<name>) $(patsubst %,-I$(UPDIR)%/include,$(PROG_DEPPATH_<name>))

UPPROG += $(PROG_TARGET_<name>)

$(PROG_OBJDIR_<name>)%.c.o: $(PROG_SRCDIR_<name>)%.c
	$(MKDIR) -p $(@D)
	$(HOST_CC) -Werror -ffreestanding -ggdb3 -c $^ -o $@ $(PROG_INCLUDE_FLAGS_<name>)

$(PROG_OBJDIR_<name>)%.asm.o: $(PROG_SRCDIR_<name>)%.asm
	$(MKDIR) -p $(@D)
	$(HOST_AS) -felf64 $^ -o $@ $(PROG_INCLUDE_FLAGS_<name>)

$(PROG_TARGET_<name>): $(PROG_OBJ_<name>) $(PROG_DEP_<name>)
	$(MKDIR) -p $(@D)
	$(HOST_LD) -o $@ $^

$(INITRD_SYSROOT)bin/<name>: $(PROG_TARGET_<name>)
	$(MKDIR) -p $(@D)
	$(CP) $^ $@

.PHONY: up-<name>
up-<name>: $(PROG_TARGET_<name>)