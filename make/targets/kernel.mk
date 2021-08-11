# This file tells make how to generate the kernel binary file from object files

$(KOUT): $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS) $(ARCHOUT) $(LINKER_SCRIPT)
	@echo Linking
	@$(LD) $(KOUT) $(LDFLAGS) $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS)
	@$(ISMULTIBOOT) $(KOUT) || (echo Output file is not multiboot complient; exit 1)

.PHONY: kernel
kernel: $(KOUT)
