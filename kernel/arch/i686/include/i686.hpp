#pragma once

#include <support/attributes.hpp>
#include <memory.hpp>

#define ARCH_TEXT   SECTION(arch.text)
#define ARCH_DATA   SECTION(arch.data)
#define ARCH_RODATA SECTION(arch.rodata)
#define ARCH_BSS    SECTION(arch.bss)


namespace i686
{
    mem::std_early_heap* early_heap();
    void init();
}
