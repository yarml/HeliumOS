#pragma once

// Compiler specific macros should be tested this way
// #if COMPILER_MACRO
// for example
// #if GCC_COMPATIBLE

#if (defined(__GNUC__) \
  || defined(__GNUG__)  \
  || defined(__clang__)) \
 && !defined(__INTEL_COMPILER)
#define GCC_COMPATIBLE 1
#elif defined(_MSC_VER)
#define MSVC_COMPILER 1
#elif defined(__INTEL_COMPILER)
#define INTEL_COMPILER 1
#endif
