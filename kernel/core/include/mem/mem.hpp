#pragma once

#include <config/early_heap.hpp>
#include <init/multiboot.h>

namespace mem
{
    void init(multiboot_info_t* mbt);
}
