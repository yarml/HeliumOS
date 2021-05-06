#ifndef UTILS_HPP
#define UTILS_HPP

#include <stddef.h>
#include <stdint.h>
#include <memory/memory.hpp>
#include <algorithm>
#include <string.h>

namespace kutils
{
    // A List that uses pages as its minimum allocation units
    template <typename T>
    class page_list
    {
    private:
        uint32_t m_count; // Allocated pages
        uint32_t m_size; // Number of entries
        uint32_t m_pstart; // Start page
    public:
        page_list()
            : m_count(1),
              m_size(0),
              m_pstart(memory::kallocvp())
        {
            if(m_pstart == INVALID_PAGE)
            {
                dbg << "Not enough Memory\n";
                m_count = 0;
            }
        }
    public:
        bool add(const T& t)
        {
            if(m_count * 4096 < sizeof(T)* (m_size + 1))
            {
                if(!realloc(2 * m_count))
                    return false;
            }
            start()[m_size++] = t;
            return true;
        }
        T* start()
        {
            return (T*)(m_pstart * 4096);
        }
        bool remove(uint32_t index)
        {
            if(index >= m_size)
                return false;
            memcpy(ptrto(index + 1), ptrto(index), sizeof(T) * (m_size - index - 1));
            m_size--;
            return true;
        }
        T* ptrto(uint32_t index)
        {
            if(index >= m_size)
                return nullptr;
            return &(start()[index]);
        }
        T& at(uint32_t index)
        {
            return *ptrto(index);
        }
        bool realloc(uint32_t count)
        {
            uint32_t new_start = memory::kallocvp(count);
            if(new_start == INVALID_PAGE)
            {
                dbg << "Cannot reallocate, not enough memory\n";
                return false;
            }
            memcpy((T*) (new_start * 4096), start(), std::min(m_size * sizeof(T), count * 4096));
            memory::freevp(m_pstart, m_count);
            m_count = count;
            m_pstart = new_start;
            return true;
        }
        uint32_t size()
        {
            return m_size;
        }
        uint32_t count()
        {
            return m_count;
        }
    public:
        bool operator+=(const T& t)
        {
            return add(t);
        }
        bool operator-=(uint32_t index)
        {
            return remove(index);
        }
        T& operator[](uint32_t index)
        {
            return at(index);
        }
    };

    size_t uint32_length(uint32_t num                  );
    void   uint32_to_string_dec(uint32_t num, char* buf);
    void   string_reverse(char* str                    );
}

#endif /* UTILS_HPP */