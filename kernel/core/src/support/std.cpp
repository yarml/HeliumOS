// std.cpp
// also know as sexually transmiteed disease.cpp
// jk it stands for standard

// This file isn't really supposed to have a 

#include <stddef.h>
#include <utils/types.hpp>

extern "C"
utils::ptr memcpy(capi::byte* to, capi::byte* const from, size_t size)
{
    for(size_t i = 0; i < size; ++i)
        to[i] = from[i];
    return to;
}
