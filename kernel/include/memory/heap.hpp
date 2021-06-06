#ifndef HEAP_HPP
#define HEAP_HPP

#include <stdint.h>
#include <kutils.hpp>
#include <memory/page_list.hpp>

namespace memory::heap
{
    class palloc_data;

    struct ptr_size
    {
        void* ptr;
        uint32_t size;
    };

    class palloc_data
    {
    private:
        uint8_t m_bitmap[512];
        uint32_t m_page;
        page_list<ptr_size> ptr_map;
    public:
        palloc_data(uint32_t page);
    public:
        bool has(uint32_t count);
        bool is_available(uint32_t offset);
        void* mark_next(uint32_t count);
        void set(uint32_t offset);
        void unset(uint32_t offset);
        void unmark(void* ptr);
        bool has_ptr(void* ptr);
    };

    void* knew(uint32_t count);
    void kfree(void* ptr);

}

#endif /* HEAP_HPP */