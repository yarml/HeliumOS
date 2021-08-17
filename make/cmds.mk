.PHONY: all bochs qemu qemu-gdb gdb clean nothing

.DEFAULT_GOAL := all

$(info Makefile version: $(MAKEFILEVER))

all: iso
	$(info Done)

nothing:

bochs: $(ISORUN) $(RDIR)/$(BOCHS_CONFIG)
	@$(CD) run && $(BOCHS) $(BOCHS_FLAGS) -f $(BOCHS_CONFIG) -rc $(BOCHSRC)

qemu: $(ISORUN)
	@$(CD) run && $(QEMU) $(QEMU_FLAGS) -cdrom $(ISO)

qemu-gdb: $(ISORUN)
	@$(CD) run && $(QEMU) $(QEMU_FLAGS) -s -S -cdrom $(ISO)

gdb: $(ISORUN)
	@$(CD) run && gdb

clean:
	$(info Removing build directory)
	@$(RM) $(BDIR)
	$(info Removing unnecessary BOCHS files)
	@$(RM) $(BOCHSLOG) $(ISORUN) $(BOCHSCOM) $(BOCHSCOPY)
	$(info Removing output template files)
	@$(RM) $(OUTPUT_TEMPLATES)
	$(info Removing additional files)
	@$(RM) $(TOCLEAN)
	$(info Cleaning iso directory)
	@$(RM) $(ISOBOOT)/$(KERNEL)
