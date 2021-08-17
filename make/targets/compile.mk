# This file tells make how to generate object files from source files

$(OBJD)/%$(CEXT)$(OEXT): %$(CEXT)
	$(info Compiling C source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(CC) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<

$(OBJD)/%$(CXXEXT)$(OEXT): %$(CXXEXT)
	$(info Compiling C++ source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(CXX) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<

$(OBJD)/%$(ASEXT)$(OEXT): %$(ASEXT)
	$(info Assembling source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
	@$(AS) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(ASFLAGS) $<
