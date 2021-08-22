# This file tells make how to generate object files from source files


$(OBJD)/%$(CEXT)$(OEXT): %$(CEXT)
	$(info Compiling C source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
ifneq (,$(findstring $(KERNEL_CORE),$@))
	@$(CC) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<
else
	@$(CC) $(OBJD)/$<$(OEXT) $(ARCH_INCLUDE_FLAGS) $(CCFLAGS) $(CFLAGS) $<
endif

$(OBJD)/%$(CXXEXT)$(OEXT): %$(CXXEXT)
	$(info Compiling C++ source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
ifneq (,$(findstring $(KERNEL_CORE),$@))
	@$(CXX) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<
else
	@$(CXX) $(OBJD)/$<$(OEXT) $(ARCH_INCLUDE_FLAGS) $(CCFLAGS) $(CXXFLAGS) $<
endif

$(OBJD)/%$(ASEXT)$(OEXT): %$(ASEXT)
	$(info Assembling source file: $<)
	@$(MKDIR) $(dir $(OBJD)/$<$(OEXT))
ifneq (,$(findstring $(KERNEL_CORE),$@))
	@$(AS) $(OBJD)/$<$(OEXT) $(INCLUDE_FLAGS) $(ASFLAGS) $<
else
	@$(AS) $(OBJD)/$<$(OEXT) $(ARCH_INCLUDE_FLAGS) $(ASFLAGS) $<
endif