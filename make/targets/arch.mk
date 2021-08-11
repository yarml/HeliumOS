# This file tells make how to generate architecture specific binary from
# architecture specific objects

$(ARCHOUT): $(ARCH_BASOBJECTS) $(ARCH_BCOBJECTS) $(ARCH_BCXXOBJECTS)
	@echo Generating architecture specific archive
	@$(MKDIR) $(dir $(ARCHOUT))
	@$(SL) $(SLFLAGS) $(ARCHOUT) $(ARCH_BASOBJECTS) $(ARCH_BCOBJECTS) $(ARCH_BCXXOBJECTS)

.PHONY: arch
arch: $(ARCHOUT)
