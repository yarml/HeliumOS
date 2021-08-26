#pragma once

#include <support/compilers.hpp>

// SECTION is the attribute used
// to force a function/variable into 
// a specific section
#if GCC_COMPATIBLE
#define SECTION(s) __attribute__((section(#s)))
#define PACKED     __attribute__((packed))
#elif MSVC_COMPILER
#define SECTION(s) __declspec(allocate(#s))
// TODO: what is the equivalent of packed for MSVC
#endif

// Standard C++ attributes

#define UNUSED [[maybe_unused]]

// TODO: support more compilers
