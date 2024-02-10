INITRD_UPPROG := $(patsubst %,$(INITRD_SYSROOT)bin/%,$(notdir $(UPPROG)))
INITRD_UPLIB := $(patsubst %,$(INITRD_SYSROOT)lib/%,$(notdir $(UPLIB)))

INITRD_UPDEP := $(INITRD_UPPROG) $(INITRD_UPLIB)
