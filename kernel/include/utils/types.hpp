#pragma once

#include <stddef.h>
#include <capi/types.hpp>


namespace utils
{
    using large_int = unsigned long long;
    using ptr       = void*             ;
    using bit_index = size_t            ;
    using size_type = size_t            ;

    template <typename t>
    class type_meta
    {
    public:
        using type = t;
    public:
        type_meta() {}
    public:
        constexpr static size_t size() { return sizeof(type); }
    };
}