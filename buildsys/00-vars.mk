MKDIR := mkdir
CURL  := curl
FIND  := find
MAKE  := make -j4
TOUCH := touch
CUT   := cut
TAR   := tar
CD 	  := cd
LS    := ls
MV    := mv
RM    := rm

ROOT_DIR 	  := $(shell pwd -P)

EXT_DIR 	  := $(ROOT_DIR)/ext/
EXT_SRC_DIR   := $(EXT_DIR)/src/
EXT_BUILD_DIR := $(EXT_DIR)/build/

SYSROOTS 	   := $(ROOT_DIR)/sysroots/
INITRD_SYSROOT := $(SYSROOTS)/initrd/
BUILD_SYSROOT  := $(SYSROOTS)/build/
HOST_SYSROOT   := $(SYSROOTS)/host/

BUILD_DIR := $(ROOT_DIR)/build

CLEAN := build

.PHONY: nothing
nothing:
