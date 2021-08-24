#pragma once

#include <utils/array.hpp>
#include "types.hpp"

namespace capi
{
    constexpr utils::array<mem_size, 2> MEMORY_UNIT_SIZES{ 4096, 4 * 1024 * 1024 };
}
