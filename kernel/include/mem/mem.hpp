#pragma once

#include <config/early_heap.hpp>
#include <init/multiboot.h>

namespace mem
{
    namespace early_heap
    {
        constexpr uint32_t BITMAP_SIZE(uint32_t heap_size, uint32_t unit_size)
        {
            return heap_size / unit_size / 8;
        }
        extern uint8_t early_heap[SIZE];
        extern uint8_t early_heap_bitmap[BITMAP_SIZE(SIZE, UNIT_SIZE)];
        class early_heap
        {
        public:
            early_heap();
            early_heap(uint8_t* heap, uint8_t* bitmap, uint32_t size, uint32_t unit_size);
        public:
            void* alloc(uint32_t size);
            void free(void* ptr, uint32_t size);
            void* realloc(void* ptr, uint32_t size, uint32_t new_size);
        private:
            bool available(uint32_t unit);
            void mark_unit(uint32_t unit);
            void unmark_unit(uint32_t unit);
        private:
            uint8_t* m_heap;
            uint8_t* m_bitmap;
            uint32_t m_size;
            uint32_t m_unit_size;
        };
    }
    void init(multiboot_info_t* mbt);
}
