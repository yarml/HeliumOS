
MKDIR := mkdir
CD 	  := cd
MAKE  := make -j4
CURL  := curl
TAR   := tar
MV    := mv
RM    := rm

ROOT_DIR 	  := $(shell pwd)

EXT_DIR 	  := $(ROOT_DIR)/ext/
EXT_SRC_DIR   := $(EXT_DIR)/src/
EXT_BUILD_DIR := $(EXT_DIR)/build/

SYSROOTS 	  := $(ROOT_DIR)/sysroots/
BUILD_SYSROOT := $(SYSROOTS)/build/
HOST_SYSROOT := $(SYSROOTS)/host/
INITRD_SYSROOT := $(SYSROOTS)/initrd/

BUILD_DIR := $(ROOT_DIR)/build
