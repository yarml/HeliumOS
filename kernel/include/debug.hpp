#pragma once

#include <capi/types.hpp>
#include <capi/io_interface.hpp>
#include <stddef.h>

extern void dbg_char(capi::io_interface const* io, char c);
extern void dbg_str(capi::io_interface const* io, char const* str);
extern void dbg_uint(capi::io_interface const* io, size_t val, size_t base);

namespace debug
{
    constexpr capi::ioadr BOCHS_DBG_PORT = 0xE9;
}
