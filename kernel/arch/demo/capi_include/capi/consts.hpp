#pragma once

// This file is supposed to define
// a number of constants that must
// be defined by each architecture

// Each variable here should be documented

#include <capi/constexpr_array.hpp>

namespace capi
{
    /// Lists the number of possible memory units(also know as pages) sizes
    /// For example the i686 architecture has 4KiB and 4MiB pages
    /// The values should be bytes
    constexpr constexpr_array<int, 2> MEMORY_UNIT_SIZES{ 4096, 4 * 1024 * 1024};
}
