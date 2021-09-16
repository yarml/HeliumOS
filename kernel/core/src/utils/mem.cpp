#include <utils/mem.hpp>

namespace mem
{
    void copy(utils::ptr dest, utils::ptr src, utils::size_type size)
    {
        for(utils::size_type i = 0; i < size; ++i)
            reinterpret_cast<capi::byte*>(dest)[i] 
                = reinterpret_cast<capi::byte*>(src)[i];
    }
    void set(utils::ptr dest, capi::byte b, utils::size_type size)
    {
        for(utils::size_type i = 0; i < size; ++i)
            reinterpret_cast<capi::byte*>(dest)[i] = b;
    }
}