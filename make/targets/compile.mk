# This file tells make how to generate object files from source files

$(OBJD)/%$(CEXT)$(OEXT): %$(CEXT)
	@echo Compiling C source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(CC) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<

$(OBJD)/%$(CXXEXT)$(OEXT): %$(CXXEXT)
	@echo Compiling C++ source file: $<
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(CXX) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<

$(OBJD)/%$(ASEXT)$(OEXT): %$(ASEXT)
	@echo Assembling: $<
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(AS) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(ASFLAGS) $<
