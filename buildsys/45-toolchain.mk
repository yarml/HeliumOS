# Toolchain to build targets for the host machine (The build system will build one if the default is left)
HOST_CC      := $(GCC_BIN)
HOST_LD      := $(BINUTILS_TOOLCHAIN_PREFIX)-ld
HOST_STRIP   := $(BINUTILS_TOOLCHAIN_PREFIX)-strip
HOST_OBJCOPY := $(BINUTILS_TOOLCHAIN_PREFIX)-objcopy
HOST_AR      := $(BINUTILS_TOOLCHAIN_PREFIX)-ar
HOST_AS      := nasm # The stock nasm is enough to build HeliumOS