#include <memory/heap.hpp>
#include <kutils.hpp>
#include <stddef.h>

namespace memory::heap
{
    palloc_data::palloc_data(uint32_t page)
        : m_page(page),
          ptr_map()
    {
        for(uint32_t i = 0; i < 512; i++)
            m_bitmap[i] = 0;
    }

    bool palloc_data::has(uint32_t count)
    {
        uint32_t counter = 0;
        for(uint32_t i = 0; i < 4096; i++)
            if(is_available(i))
            {
                ++counter;
                if(counter == count)
                    return true;
            }
        return false;
    }
    bool palloc_data::is_available(uint32_t offset)
    {
        if(offset >= 4096)
            return false;
        uint8_t bit = offset % 8;
        uint32_t byte = (offset - bit) / 8;
        bool val = (m_bitmap[byte] >> bit) & 1;
        return !val;
    }
    void* palloc_data::mark_next(uint32_t count)
    {
        uint32_t start = (uint32_t) -1;
        uint32_t counter = 0;
        for(uint32_t i = 0; i < 4096; i++)
        {
            if(is_available(i))
            {
                if(start > 4096)
                    start = i;
                ++counter;
                if(counter == count)
                {
                    for(uint32_t j = start; j < start + count; j++)
                        set(j);
                    void* ptr = (void*) (m_page * 4096 + start);
                    ptr_map += ptr_size{ .ptr = ptr, .size = count };
                    return ptr;
                }
            }
            else
            {
                counter = 0;
                start = -1;
            }
        }
        return nullptr;
    }
    void palloc_data::set(uint32_t offset)
    {
        uint8_t bit = offset % 8;
        uint32_t byte = (offset - bit) / 8;
        m_bitmap[byte] |= 1 << bit;
    }
    void palloc_data::unset(uint32_t offset)
    {
        uint8_t bit = offset % 8;
        uint32_t byte = (offset - bit) / 8;
        m_bitmap[byte] &= ~(1 << bit);
    }
    bool palloc_data::has_ptr(void* ptr)
    {
        for(uint32_t i = 0; i < ptr_map.size(); i++)
        {
            ptr_size& ps = ptr_map[i];
            if(ps.ptr == ptr)
            {
                return true;
            }
        }
        return false;
    }
    void palloc_data::unmark(void* ptr)
    {
        uint32_t count = -1;
        uint32_t index = -1;
        for(uint32_t i = 0; i < ptr_map.size(); i++)
        {
            ptr_size& ps = ptr_map[i];
            if(ps.ptr == ptr)
            {
                count = ps.size;
                index = i;
                break;
            }
        }
        if(index != (uint32_t) -1)
        {
            ptr_map -= index;
            for(uint32_t i = (uint32_t)(ptr) - m_page * 4096; i < count; i++)
                unset(i);
        }
        else
            unset((uint32_t)(ptr) - m_page * 4096);
    }

    static page_list<palloc_data> s_palloc_data;
    // page_list<ptr_size> s_alloc_journal;
    
    void* knew(uint32_t count)
    {
        if(count > 4096)
            dbg << "Allocations for blocks of memory larger than 4096 bytes is not supported yet\n";
        for(uint32_t i = 0; i < s_palloc_data.size(); i++)
        {
            palloc_data& p = s_palloc_data[i];
            if(p.has(count))
            {
                void* ptr = p.mark_next(count);
                //s_alloc_journal += { ptr, count};
                dbg << "Allocated: " << ptr << ':' << count << '\n';
                return ptr;
            }
        }
        s_palloc_data += palloc_data(kallocvp());
        void* ptr = s_palloc_data.last().mark_next(count);
        //s_alloc_journal += { ptr, count };
        dbg << "Allocated: " << ptr << ':' << count << '\n';
        return ptr;
    }
    void kfree(void* ptr)
    {
        for(uint32_t i = 0; i < s_palloc_data.size() +1; i++)
        {
            palloc_data& p = s_palloc_data[i];
            if(p.has_ptr(ptr))
            {
                p.unmark(ptr);
                return;
            }
        }
        dbg << "Tried to free unallocated pointer " << ptr << '\n';
    }

    void list_journal()
    {
        // dbg << "Allocation journal\n";
        // for(uint32_t i = 0; i < s_alloc_journal.size(); i++)
        // {
        //     ptr_size& ps = s_alloc_journal[i];
        //     dbg << ps.ptr << ':' << ps.size << '\n';
        // }
        // dbg << "End of Allocation Journal\n";
    }
}
