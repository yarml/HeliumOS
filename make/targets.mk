$(ISORUN): $(KOUT) $(GRUBCFG)
	@echo Generating ISO file for the OS
	@$(MKDIR) $(ISOBOOT)
	@$(CP) $(KOUT) $(ISOBOOT)/$(KERNEL)
	@$(MKDIR) $(RDIR)
	@$(MKISO) $(ISORUN) $(ISODIR)

$(KOUT): $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS) $(LINKER_SCRIPT)
	@echo Linking
	@$(LD) $(KOUT) $(LDFLAGS) $(BASOBJECTS) $(BCOBJECTS) $(BCXXOBJECTS)
	@$(ISMULTIBOOT) $(KOUT) || (echo Output file is not multiboot complient; exit 1)

$(OBJD)/%$(CEXT).o: %$(CEXT)
	@echo Compiling C source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(CC) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<

$(OBJD)/%$(CXXEXT).o: %$(CXXEXT)
	@echo Compiling C++ source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(CXX) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<

$(OBJD)/%$(ASEXT).o: %$(ASEXT)
	@echo Assembling: $<
	@$(MKDIR) $(dir $(OBJD)/$<.o)
	@$(AS) $(OBJD)/$<.o $(INCLUDE_FLAGS) $(ASFLAGS) $<
