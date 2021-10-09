// std.cpp
// also know as sexually transmiteed disease.cpp
// jk it stands for standard.cpp

// This file isn't really supposed to have a header
// It's just here so that the linker can shut up about
// missing functions like memcpy

#include <stddef.h>
#include <utils/types.hpp>
#include <utils/mem.hpp>

extern "C"
{
    using function = void (*)(void);

    utils::ptr memcpy(capi::byte* to, capi::byte* const from, utils::size_type size)
    {
        mem::copy(to, from, size);
        return to;   
    }

    // That fucking C standard requires an integer as the second argument
    // Why tf is that?
    utils::ptr memset(utils::ptr dest, int b, utils::size_type size)
    {
        mem::set(dest, b, size);
        return dest;
    }

    int atexit(function)
    { 
        return 0;
    }
}
