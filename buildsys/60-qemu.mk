
.PHONY: run-qemu
run-qemu: $(HELIUM_IMG)
	$(QEMU_BIN) $(QEMU_FLAGS) -drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
		-drive if=pflash,format=raw,unit=1,file=$(OVMF_VARS) \
		$(HELIUM_IMG)

