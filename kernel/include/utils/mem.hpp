#pragma once

#include <utils/types.hpp>

namespace mem
{
    void copy(utils::ptr dest, utils::ptr src, utils::size_type size);
    void set(utils::ptr dest, capi::byte b, utils::size_type size);
}
