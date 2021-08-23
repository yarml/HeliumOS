#pragma once

// This file is supposed to define
// a number of types that must
// be defined by each architecture
#include <stdint.h>

namespace capi
{
    typedef uint8_t  byte    ;
    typedef uint16_t word    ;
    typedef uint32_t dword   ;
    typedef uint32_t qword   ;
    /// Used to index memory blocks/units/pages
    typedef dword    blockidx;
    typedef dword    adr     ;
}
