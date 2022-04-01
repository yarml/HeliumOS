# Don't include binaries we don't use
s/memcpy(esp, binary_boot_bin, 512);//g
s/rootdir = esp_addzfile.*//g
s/ptr = esp_addzfile(rootdir.*//g
s/ptr = esp_addzfile(ptr, "\(BOOTCODE.BIN\|FIXUP.DAT\|START.ELF\|LICENCE.BCM\).*//g
