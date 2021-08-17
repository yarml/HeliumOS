# This file tells make how to generate a set of files from templates
# Templates are found in $(TEMPLATES_DIR) (default: templates/)

.PHONY: $(OUTPUT_TEMPLATES)

.SECONDEXPANSION:
$(OUTPUT_TEMPLATES): $(TEMPLATES_DIR)/$$@$(M4EXT)
	$(info Generating file from template: $@)
	@$(MKDIR) $(dir $@)
	@$(M4) $@ $(M4FLAGS) $<

