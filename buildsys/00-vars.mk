
MKDIR := mkdir
MAKE  := make -j4
FIND  := find
CURL  := curl
CUT   := cut
TAR   := tar
CD 	  := cd
MV    := mv
RM    := rm
LS    := ls

ROOT_DIR 	  := $(shell pwd)

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
