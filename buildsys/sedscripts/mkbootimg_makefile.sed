# Use the BUILD_CC instead of the hardcoded gcc
s|CC =.*|CC := #CC#|g
# Only include the bootboot.efi built for Helium
s|INCBIN =.*|INCBIN := #BOOTBOOT_BIN#|g
