#pragma once

#include <stdint.h>

namespace utils::string
{
    uint32_t uint_len(uint32_t n);
    void uint_to_str(uint32_t n, char* buf);
    void str_reverse(char* str);
    uint32_t str_len(char* str);
}
