#include <mem/mem.hpp>
#include <utils/mem.hpp>
#include <utils/math.hpp>

namespace mem::early_heap
{
    uint8_t early_heap[SIZE];
    uint8_t early_heap_bitmap[BITMAP_SIZE(SIZE, UNIT_SIZE)];
    early_heap::early_heap() { }
    early_heap::early_heap(uint8_t* heap, uint8_t* bitmap, uint32_t size, uint32_t unit_size)
     : m_heap(heap),
       m_bitmap(bitmap),
       m_size(size),
       m_unit_size(unit_size)
    { }
    void* early_heap::alloc(uint32_t size)
    {
        uint32_t units_count = size / m_unit_size;
        uint32_t available_bytes_count = 0;
        uint32_t first_unit = INVALID_UNIT;
        for(uint32_t i = 0; i < m_size / m_unit_size; ++i)
        {
            if(available(i))
            {
                available_bytes_count += m_unit_size;
                if(first_unit == INVALID_UNIT)
                    first_unit = i;
                if(available_bytes_count / m_unit_size == units_count)
                {
                    for(uint32_t j = 0; j < units_count; ++j)
                        mark_unit(j + first_unit);
                    return first_unit * m_unit_size + m_heap;
                }
            }
            else
            {
                available_bytes_count = 0;
                first_unit = INVALID_UNIT;
            }
        }
        return nullptr;
    }
    void early_heap::free(void* ptr, uint32_t size)
    {
        for(uint32_t i = 0; i < size / m_unit_size; ++i)
            unmark_unit(i + (uint32_t) ptr / m_unit_size);
    }
    void* early_heap::realloc(void* ptr, uint32_t size, uint32_t new_size)
    {
        void* new_ptr = alloc(new_size);
        if(new_ptr == nullptr)
            return nullptr;
        mem::copy(ptr, new_ptr, math::min(size, new_size));
        free(ptr, size);
        return new_ptr;
    }
    bool early_heap::available(uint32_t unit)
    {
        uint32_t bit = unit % 8;
        uint32_t byte = (unit - bit) / 8;
        return !((m_bitmap[byte] >> bit) & 1);
    }
    void early_heap::mark_unit(uint32_t unit)
    {
        uint8_t bit = unit % 8;
        uint32_t byte = (unit - bit) / 8;
        m_bitmap[byte] |= 1 << bit;
    }
    void early_heap::unmark_unit(uint32_t unit)
    {
        uint8_t bit = unit % 8;
        uint32_t byte = (unit - bit) / 8;
        m_bitmap[byte] &= ~(1 << bit);
    }
}
