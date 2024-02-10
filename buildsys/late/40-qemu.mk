
# TODO: compile qemu too?
# TODO: compile ovmf, this one is more urgent

.PHONY: run-qemu

QEMU_CMD := $(QEMU_BIN) \
		-cpu qemu64 \
		-net none \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
		-drive if=pflash,format=raw,unit=1,file=$(OVMF_VARS) \
		-drive id=sysimg,if=none,format=raw,file=$(HELIUM_IMG) \
		-device ahci,id=ahci \
		-device ide-hd,drive=sysimg,bus=ahci.0 \
		-debugcon stdio \
		-m 512M \
		-smp 4

run-qemu: bootimg
	$(QEMU_CMD)


debug: bootimg
	$(TMUX) new $(GDB) -tui \; \
		splitp -h $(QEMU_CMD) -d int,cpu_reset,guest_errors -s -S\
			-monitor unix:$(BUILD_DIR)qms,server\; \
		splitp -v $(SHELL) $(SHDIR)monitor.sh \
		"$(SOCAT) -,echo=0,icanon=0 unix-connect:$(BUILD_DIR)qms"
