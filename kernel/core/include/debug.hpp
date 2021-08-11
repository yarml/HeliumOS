#pragma once

#include <stdint.h>

namespace debug
{
    constexpr uint16_t BOCHS_OUT_PORT = 0xE9;
    void dbg_write(const char* str);
    void dbg_write(uint32_t n);
}
