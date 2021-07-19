.PHONY: all kernel iso bochs qemu qemu-gdb gdb clean

all: iso
	@echo Done

kernel: $(KOUT)
iso: $(ISORUN)

bochs: $(ISORUN)
	@cd run && $(BOCHS) $(BOCHS_FLAGS) -f $(BOCHS_CONFIG) -rc $(BOCHSRC)

qemu: $(ISORUN)
	@cd run && $(QEMU) $(QEMU_FLAGS) -cdrom $(ISO)

qemu-gdb: $(ISORUN)
	@cd run && $(QEMU) $(QEMU_FLAGS) -s -S -cdrom $(ISO)

gdb: $(ISORUN)
	@cd run && gdb

clean:
	@echo Cleaning
	@$(RM) $(TOCLEAN) $(BDIR) $(ISOBOOT)/$(KERNEL) $(BOCHSLOG) $(ISORUN) $(BOCHSCOM) $(BOCHSCOPY)
