#ifndef UTILS_HPP
#define UTILS_HPP

#include <stddef.h>
#include <stdint.h>
#include <memory/memory.hpp>
#include <kmath.hpp>

namespace kutils
{
    size_t uint32_length(uint32_t num);
    void   uint32_to_string_dec(uint32_t num, char* buf);
    void   string_reverse(char* str);
    uint32_t string_len(const char* str);
    char* string_chr(const char* str, char c);
    void* mem_copy(void* to, const void* from, uint32_t size);
    uint32_t char_pos(const char* str, char c);
    int32_t string_compare(const char* str1, const char* str2);
    char* string_copy(char* to, const char* from);
    int32_t mem_compare(const void* p1, const void* p2, uint32_t size);
}

#endif /* UTILS_HPP */