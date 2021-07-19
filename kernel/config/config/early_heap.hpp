#pragma once

#include <stdint.h>

namespace mem::early_heap
{
    /** Early heap size in bytes **/
    constexpr uint32_t SIZE = 2048;
    /** Smallest allocation unit size in bytes **/
    constexpr uint32_t UNIT_SIZE = 4;
    /** Invalid unit identifier, changing it to a very low value may (will) result in bugs
      There is no point in changing it anyway **/
    constexpr uint32_t INVALID_UNIT = 0xFF'FF'FF'FF;
}
