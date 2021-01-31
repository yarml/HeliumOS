#ifndef UTILS_HPP
#define UTILS_HPP

#include <stddef.h>
#include <stdint.h>

namespace utils
{
    size_t uint32_length(   uint32_t num           );
    void   uint32_to_string_dec(uint32_t num, char* buf);
    void   string_reverse(  char* str              );
}

#endif /* UTILS_HPP */