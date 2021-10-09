#include <memory.hpp>
#include <i686.hpp>
#include <multiboot.hpp>
#include <capi/memory.hpp>
#include <debug.hpp>


namespace i686::mem
{
    void init(capi::architecture* arch, multiboot::info_structure* mbt_info)
    {
        i686::init();
        capi::detect_memory(arch, mbt_info);
        gdt::init(arch);
    }
}
