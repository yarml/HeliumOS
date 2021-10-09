#pragma once

#include <stddef.h>
#include <utils/literals.hpp>

namespace i686::mem
{
    namespace early_heap_config
    {
        /// Minimum allocation unit in bytes, can only allocated in blocks of this multiple
        constexpr size_t UNIT = 4    ;
        /// Early heap size in bytes, preferably a multiple of UNIT
        constexpr size_t SIZE = 4_Kib;
    }
}
