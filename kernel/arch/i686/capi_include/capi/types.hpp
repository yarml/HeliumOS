#pragma once

#include <stdint.h>

namespace capi
{
    using byte      = uint8_t ;
    using word      = uint16_t;
    using dword     = uint32_t;
    using qword     = uint32_t; // maybe uint64_t? not sure, until I need it
    using pblockidx = dword   ;
	using vblockidx = dword   ;
    using adr       = dword   ;
    using mem_size  = dword   ;
    using ioadr     = word    ;
    
}
