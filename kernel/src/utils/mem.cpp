#include <utils/mem.hpp>

namespace mem
{
    void copy(void* from, void* to, uint32_t size)
    {
        for(uint32_t i = 0; i < size; ++i)
            ((uint8_t*) to)[i] = ((uint8_t*) from)[i];
    }
}
