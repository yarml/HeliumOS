# Targets to install on real hardware, must be executed as root

EFI_PARTITION := /boot
IMG_EFI_OFFSET := 65536

MOUNT_PATH := $(BUILD_DIR)mnt/

.PHONY: install

# This doesn't directly depend on HELIUM_ING and BOOTBOOT_BIN, otherwise,
# make will build them with the root user, which may cause problems
install:
	@$(TEST) -f $(BOOTBOOT_BIN) \
			|| ($(ECHO) Build the bootboot.efi first before installing using ´make bootboot´; exit 1)
	@$(TEST) -f $(HELIUM_IMG) \
			|| ($(ECHO) Build the helium.img first before installing using ´make bootimg´   ; exit 1)
	$(MKDIR) -p $(MOUNT_PATH)
	$(CP) $(BOOTBOOT_BIN) $(EFI_PARTITION)
	$(MOUNT) -o loop,offset=$(IMG_EFI_OFFSET) $(HELIUM_IMG) $(MOUNT_PATH)
	$(CP) $(MOUNT_PATH)BOOTBOOT $(EFI_PARTITION)
	$(UMOUNT) $(MOUNT_PATH)
