#pragma once

#include <stdint.h>
#include <config/early_heap.hpp>

namespace mem::early_heap
{
    constexpr uint32_t BITMAP_SIZE(uint32_t heap_size, uint32_t unit_size)
    {
        return heap_size / unit_size / 8;
    }
    extern uint8_t early_heap_area[SIZE];
    extern uint8_t early_heap_bitmap[BITMAP_SIZE(SIZE, UNIT_SIZE)];
    class early_heap
    {
    public:
        early_heap();
        early_heap(uint8_t* heap, uint8_t* bitmap, uint32_t size, uint32_t unit_size);
    public:
        void* alloc(uint32_t size = UNIT_SIZE);
        void free(void const* ptr, uint32_t size = UNIT_SIZE);
        void* realloc(void const* ptr, uint32_t size = UNIT_SIZE, uint32_t new_size = UNIT_SIZE);
    public:
        template <typename T>
        T* alloc(uint32_t size = 1)
        {
            return alloc(size * sizeof(T));
        }
        template <typename T>
        void free(T const* ptr, uint32_t size = 1)
        {
            free(ptr, size * sizeof(T));
        }
        template <typename T>
        T* realloc(T const* ptr, uint32_t size = 1, uint32_t new_size = 1)
        {
            return realloc(ptr, size * sizeof(T), new_size * sizeof(T));
        }
    public:
        bool available(uint32_t unit);
        void mark_unit(uint32_t unit);
        void unmark_unit(uint32_t unit);
    private:
        uint8_t* m_heap;
        uint8_t* m_bitmap;
        uint32_t m_size;
        uint32_t m_unit_size;
    };
    template <typename T>
    class early_list
    {
    public:
        early_list();
    public:
        void add(T const& object);
        void remove(uint32_t index);
        void insert(T const& object, uint32_t index);
    public:
        void operator+=(T const& object);
        void operator-=(uint32_t index);
    private:
        T* m_data;
        early_heap* m_heap;
        uint32_t m_size;
    };
}
