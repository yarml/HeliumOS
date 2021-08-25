#pragma once

#define GCC_COMPATIBLE            \
        (  (defined(__GNUC__)      \
        || defined(__GNUG__)        \
        || defined(__clang__))       \
        && !defined(__INTEL_COMPILER) \
        )

#define MSVC_COMPILER (defined(_MSC_VER))
#define INTEL_COMPILER (defined(__INTEL_COMPILER))
