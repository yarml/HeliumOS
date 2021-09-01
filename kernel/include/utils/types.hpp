#pragma once

namespace utils
{
    using ull       = unsigned long long;
    using ptr       = void*             ;
    using bit_index = size_t            ;

    template <typename type>
    class type_meta
    {
    public:
        type_meta() {}
    public:
        constexpr static size_t size() { return sizeof(type); }
    };
}