# Define common variables
M ?= DEBUG

QEMU_BIN   := qemu-system-x86_64
OVMF_CODE  := /usr/share/edk2-ovmf/x64/OVMF_CODE.fd
OVMF_VARS  := /usr/share/edk2-ovmf/x64/OVMF_VARS.fd
# Toolchain to build targets for the build machine
BUILD_CC := gcc
BUILD_CARGO := cargo +stable

CLANG_FORMAT := clang-format
LSB_RELEASE := lsb_release

MKFIFO := mkfifo
UMOUNT := umount
MKDIR  := mkdir
MOUNT  := mount
SHELL  := bash
TOUCH  := touch
ECHO   := echo
EXIT   := exit
SOCAT  := socat
FALSE  := false
XARGS  := xargs
CURL   := curl
FIND   := find
GZIP   := gzip -f
MAKE   := make -j4
READ   := read
TEST   := test
TMUX   := tmux
TRUE   := true
CAT    := cat
CUT    := cut
GDB    := gdb
GIT    := git
TAR    := tar
SED    := sed
CD 	   := cd
CP     := cp -rf
LS     := ls
MV     := mv -f
PY	   := python3
RM     := rm -rf
TR     := tr
WC     := wc

# if this has a problem detecting your distro,
# just put your distro name manually
DISTRO := $(shell $(LSB_RELEASE) -i | $(CUT) -d ':' -f2 | $(TR) -d " \t")

ROOT_DIR 	  := $(shell pwd -P)/

CONFIG_DIR =$(ROOT_DIR)config/

LINKER_SCRIPT  := $(CONFIG_DIR)link.ld
BOOTBOOT_CFG := $(CONFIG_DIR)bootboot.config
BOOTIMG_CFG  := $(CONFIG_DIR)bootimg.json
TRIPLET_CFG := $(CONFIG_DIR)helium.json

UPDIR := $(ROOT_DIR)userspace/

EXT_DIR 	  := $(ROOT_DIR)ext/
EXT_SRC_DIR   := $(EXT_DIR)src/
EXT_BUILD_DIR := $(EXT_DIR)build/

TOOLS_DIR := $(ROOT_DIR)tools/

SYSROOTS 	   := $(ROOT_DIR)sysroots/
INITRD_SYSROOT := $(SYSROOTS)initrd/
BUILD_SYSROOT  := $(SYSROOTS)build/
HOST_SYSROOT   := $(SYSROOTS)host/

BUILD_DIR := $(ROOT_DIR)build/
OUT_DIR := $(BUILD_DIR)out/
UPOUT_DIR := $(OUT_DIR)userspace/

BUILDSYS      := buildsys/
SEDDIR        := $(BUILDSYS)sedscripts/
IDEDIR        := $(BUILDSYS)IDE/
PYDIR         := $(BUILDSYS)pyscripts/
SHDIR         := $(BUILDSYS)shscripts/
TEMPLATES_DIR := $(BUILDSYS)template/

CACHE_DIR := $(ROOT_DIR)cache/

CLEAN := $(BUILD_DIR)

# Default target
.PHONY: nothing
nothing:

# Include rest of build system
include buildsys/*.mk

# $(shell $(MAKE) -C $(ROOT_DIR)tools/ >/dev/null)
# $(shell $(BUILD_SYSROOT)bin/upconfig >/dev/null)

# include build/buildsys/*.mk

# Include the late buildsystem
include buildsys/late/*.mk
