#pragma once

#include <support/compilers.hpp>

// SECTION_ATT is the attribute used
// to force a function/variable into 
// a specific section
#if GCC_COMPATIBLE
#define SECTION(s) __attribute__((section(#s)))
#elif MSVC_COMPILER
#define SECTION(s) __declspec(allocate(#s))
#endif

// TODO: support more compilers
