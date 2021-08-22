#pragma once


namespace capi
{
    template <class T, unsigned int len>
    struct constexpr_array
    {
    public:
        T[len] m_array;
    };
}