#include <memory/heap.hpp>
#include <kutils.hpp>
#include <stddef.h>

namespace memory::heap
{
    debug& operator<<(debug& d, const palloc_data& ps)
    {
        d  << "pallocdata: " << ps.m_page;
        return d;
    }

    debug& operator<<(debug& d, const ptr_size& ps)
    {
        d << "ptr_size: " << ps.ptr << ' ' << ps.size;
        return d;
    }

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
                    dbg << "page: " << m_page << " registers pointer: " << ptr << " size: " << count << '\n';
                    ptr_map += ptr_size{ .ptr = ptr, .size = count };
                    for(uint32_t j = 0; j < ptr_map.size(); j++)
                        dbg << ptr_map[j].ptr << ' ' << ptr_map[j].size << '\n';
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
        dbg << "Looking for: " << ptr << " in " << m_page << '\n';
        dbg << "ptr_map size " << ptr_map.size() << '\n';
        for(uint32_t i = 0; i < ptr_map.size(); i++)
        {
            ptr_size& ps = ptr_map[i];
            dbg << ptr_map[i].ptr << ' ' << ptr_map[i].size << '\n';
            if(ps.ptr == ptr)
            {
                dbg << "page: " << m_page << " has " << ptr << '\n';
                return true;
            }
        }
        dbg << "page: " << m_page << " does not have " << ptr << '\n';
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

    static kutils::page_list<palloc_data> s_palloc_data;

    void* knew(uint32_t count)
    {
        if(count > 4096)
            dbg << "Allocations for blocks of memory larger than 4096 bytes is not supported yet\n";
        dbg << "palloc_data list size: " << s_palloc_data.size() << '\n';
        for(uint32_t i = 0; i < s_palloc_data.size(); i++)
        {
            palloc_data& p = s_palloc_data[i];
            if(p.has(count))
            {
                void* ptr = p.mark_next(count);
                dbg << "Allcated: " << ptr << ' ' << count << '\n';
                return ptr;
            }
        }
        uint32_t new_page = kallocvp();
        if(new_page == 0)
            dbg << "Got a page in 000000000000000000000000000000000000";
        s_palloc_data += palloc_data(new_page);
        void* ptr = s_palloc_data.last().mark_next(count);
        dbg << "palloc_data list size: " << s_palloc_data.size() << '\n';
        dbg << "New page: " << new_page << " Allcated: " << ptr << ' ' << count << '\n';
        return ptr;
    }
    void kfree(void* ptr)
    {
        dbg << "palloc_data list size: " << s_palloc_data.size() << '\n';
        for(uint32_t i = 0; i < s_palloc_data.size() +1; i++)
        {
            palloc_data& p = s_palloc_data[i];
            dbg << "Trying for " << p << '\n';
            if(p.has_ptr(ptr))
            {
                p.unmark(ptr);
                return;
            }
        }
        dbg << "Tried to free unallocated pointer " << ptr << '\n';
    }
}
