#include <memory.hpp>
#include <debug.hpp>
#include <utils/maths.hpp>
#include <multiboot.h>
#include <i686.hpp>
#include <capi/early_ordered_list.hpp>
#include <multiboot.hpp>
#include <capi/memory.hpp>

namespace i686::mem
{
    void init(capi::architecture* arch, multiboot::info_structure* mbt_info)
    {
        capi::detect_memory(arch, mbt_info);
        gdt::init(arch);
    }
}