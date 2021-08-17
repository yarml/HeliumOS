# This file tells make how to generate the iso file from the kernel binary

$(ISORUN): $(KOUT) $(GRUBCFG)
	$(info Generating ISO file for the OS)
	@$(CP) $(KOUT) $(ISOBOOT)/$(KERNEL)
	@$(MKISO) $(ISORUN) $(ISODIR)

.PHONY: iso
iso: $(ISORUN)
