#ifndef UTILS_HPP
#define UTILS_HPP

#include <stddef.h>
#include <stdint.h>
#include <memory/memory.hpp>
#include <kmath.hpp>

namespace kutils
{
    size_t uint32_length(uint32_t num);
    void   uint32_to_string_dec(uint32_t num, char* buf);
    void   string_reverse(char* str);
    uint32_t string_len(const char* str);
    char* string_chr(const char* str, char c);
    void* mem_copy(void* to, const void* from, uint32_t size);
    uint32_t char_pos(const char* str, char c);
    uint32_t string_compare(const char* str1, const char* str2);
    char* string_copy(char* to, const char* from);

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
            : m_count(0),
              m_size(0),
              m_pstart(INVALID_PAGE)
        {
            dbg << "Calling a page_list constructor\n";
        }
    public:
        bool add(const T& t)
        {
            dbg << "Adding new element to page list\n";
            if(m_count * 4096 < sizeof(T) * (m_size + 1))
            {
                dbg << "We need to reallocate because: " << m_count * 4096 << ' ' << sizeof(T) * (m_size+1) <<  "\n";
                if(!realloc(2 * m_count))
                    return false;
            }
            uint32_t old_size = m_size;
            start()[m_size++] = t;
            dbg << "Old size: " << old_size << " new size: " << m_size << '\n';
            dbg << "---------------------------------------\n";
            for(uint32_t i = 0; i < m_size; i++)
                dbg << start()[i] << '\n';
            dbg << "---------------------------------------\n";
            return true;
        }
        T* start() const
        {
            return (T*)(m_pstart * 4096);
        }

        T& first() const
        {
            return *start();
        }

        T& last() const
        {
            return at(m_size - 1);
        }

        bool remove(uint32_t index)
        {
            if(index >= m_size)
                return false;
            mem_copy(ptrto(index), ptrto(index + 1), sizeof(T) * (m_size - index - 1));
            m_size--;
            return true;
        }
        T* ptrto(uint32_t index) const
        {
            if(index >= m_size)
                return nullptr;
            return &(start()[index]);
        }
        T& at(uint32_t index) const
        {
            return *ptrto(index);
        }
        bool realloc(uint32_t count)
        {
            count = count != 0 ? count : 1;
            uint32_t new_start = memory::kallocvp(count);
            if(new_start == INVALID_PAGE)
            {
                dbg << "Cannot reallocate, not enough memory\n";
                return false;
            }
            kutils::mem_copy((T*) (new_start * 4096), start(), kmath::min(m_size * sizeof(T), (count * 4096) / sizeof(T)));
            memory::freevp(m_pstart, m_count);
            m_count = count;
            m_pstart = new_start;
            uint32_t old_size = m_size;
            dbg << (m_size * sizeof(T) < m_count * 4096) << " count: " << m_count << '\n';
            m_size = m_size * sizeof(T) <= m_count * 4096 ? m_size : (m_count * 4096) / sizeof(T);
            dbg << "realloc old size: " << old_size << " new size: " << m_size << " size of T " << sizeof(T) << '\n';
            return true;
        }
        uint32_t size() const
        {
            return m_size;
        }
        uint32_t count() const
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
        T& operator[](uint32_t index) const
        {
            return at(index);
        }
    };
}

#endif /* UTILS_HPP */