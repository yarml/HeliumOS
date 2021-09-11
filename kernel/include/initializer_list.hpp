#pragma once

#include <support/compilers.hpp>

#include <stddef.h>

namespace std // I'm forced to :pensive:
{
    template <typename t>
    class initializer_list
    {
    public:
        using value_type      = t       ;
        using reference       = t const&;
        using const_reference = t const&;
        using size_type       = size_t  ;
        using iterator        = t const*;
        using const_iterator  = t const*;
    public:
        constexpr initializer_list() 
            : m_array(nullptr),
              m_size(0)
        { }
        constexpr size_type size()
        {
            return m_size;
        }
        constexpr const_iterator begin() const
        {
            return m_array;
        }
        constexpr const_iterator end() const
        {
            return m_array + m_size;
        }
    private:
        constexpr initializer_list(const_iterator array, size_type len)
            : m_array(array),
              m_size(len)
        { }
    private:
        iterator  m_array;
        size_type m_size ;
    };
}
