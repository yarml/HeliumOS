
INCLUDE_FLAGS := $(patsubst %,-I%,$(INCLUDES))

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))


ASSOURCES  := $(call rwildcard,$(SRCD),*$(ASEXT))
CSOURCES   := $(call rwildcard,$(SRCD),*$(CEXT))
CXXSOURCES := $(call rwildcard,$(SRCD),*$(CXXEXT))

ASOBJECTS  := $(patsubst %,%.o,$(ASSOURCES))
COBJECTS   := $(patsubst %,%.o,$(CSOURCES))
CXXOBJECTS := $(patsubst %,%.o,$(CXXSOURCES))

BASOBJECTS  := $(patsubst %,$(OBJD)/%,$(ASOBJECTS))
BCOBJECTS   := $(patsubst %,$(OBJD)/%,$(COBJECTS))
BCXXOBJECTS := $(patsubst %,$(OBJD)/%,$(CXXOBJECTS))
