
MAKEFILEVER := 0.2


INCLUDE_FLAGS := $(patsubst %,-I%,$(INCLUDES))
ARCH_INCLUDE_FLAGS := $(patsubst %,-I%,$(ARCH_INCLUDES))

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASSOURCES  := $(call rwildcard,$(SRCD),*$(ASEXT))
CSOURCES   := $(call rwildcard,$(SRCD),*$(CEXT))
CXXSOURCES := $(call rwildcard,$(SRCD),*$(CXXEXT))

ARCH_ASSOURCES  := $(call rwildcard,$(ARCH_SRC),*$(ASEXT))
ARCH_CSOURCES   := $(call rwildcard,$(ARCH_SRC),*$(CEXT))
ARCH_CXXSOURCES := $(call rwildcard,$(ARCH_SRC),*$(CXXEXT))

ASOBJECTS  := $(patsubst %,%$(OEXT),$(ASSOURCES))
COBJECTS   := $(patsubst %,%$(OEXT),$(CSOURCES))
CXXOBJECTS := $(patsubst %,%$(OEXT),$(CXXSOURCES))

ARCH_ASOBJECTS := $(patsubst %,%$(OEXT),$(ARCH_ASSOURCES))
ARCH_COBJECTS   := $(patsubst %,%$(OEXT),$(ARCH_CSOURCES))
ARCH_CXXOBJECTS := $(patsubst %,%$(OEXT),$(ARCH_CXXSOURCES))

BASOBJECTS  := $(patsubst %,$(OBJD)/%,$(ASOBJECTS))
BCOBJECTS   := $(patsubst %,$(OBJD)/%,$(COBJECTS))
BCXXOBJECTS := $(patsubst %,$(OBJD)/%,$(CXXOBJECTS))

ARCH_BASOBJECTS  := $(patsubst %,$(OBJD)/%,$(ARCH_ASOBJECTS))
ARCH_BCOBJECTS   := $(patsubst %,$(OBJD)/%,$(ARCH_COBJECTS))
ARCH_BCXXOBJECTS := $(patsubst %,$(OBJD)/%,$(ARCH_CXXOBJECTS))

TEMPLATE_FILES   := $(call rwildcard,$(TEMPLATES_DIR),*$(M4EXT))
OUTPUT_TEMPLATES := $(patsubst $(TEMPLATES_DIR)/%$(M4EXT),%,$(TEMPLATE_FILES))
