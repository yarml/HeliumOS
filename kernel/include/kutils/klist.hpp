#ifndef KLIST_HPP
#define KLIST_HPP

#include <stdint.h>
#include <kutils.hpp>

namespace kutils
{
    template <typename T>
    class klist
    {
    private:
        T* m_buffer;
        uint32_t m_size;  // Number of elements
        uint32_t m_count; // Max number of elements before reallocating
    public:
        klist()
            : m_buffer(nullptr),
              m_size(0),
              m_count(0)
        {
        }
        ~klist()
        {
            dbg << "Destructing a klist\n";
            if(m_buffer != nullptr)
                delete[] m_buffer;
        }
    public:
        bool add(const T& t)
        {
            if(m_size >= m_count)
                if(!realloc(2 * m_count))
                    return false;
            m_buffer[m_size++] = t;
            return true;
        }
        bool remove(uint32_t index)
        {
            if(index >= m_size)
                return false;
            kutils::mem_copy(ptrto(index), ptrto(index + 1), sizeof(T) * (m_size - index - 1));
            m_size--;
            return true;
        }
        T* ptrto(uint32_t index) const
        {
            if(index >= m_size)
                return nullptr;
            return &(m_buffer[index]);
        }
        T* buffer() const
        {
            return m_buffer;
        }
        T& first() const
        {
            return m_buffer[0];
        }
        T& last() const
        {
            return m_buffer[m_size - 1];
        }
        uint32_t size() const
        {
            return m_size;
        }
        uint32_t count() const
        {
            return m_count;
        }
        T& at(uint32_t index) const
        {
            return m_buffer[index];
        }
        bool realloc(uint32_t new_count)
        {
            new_count = new_count != 0 ? new_count : 4;
            if(new_count == m_size)
                return true;
            T* new_buffer = new T[new_count];
            if(new_buffer == nullptr)
            {
                dbg << "Not enough memory to reallocate\n";
                return false;
            }
            kutils::mem_copy(new_buffer, m_buffer, kmath::min(m_size * sizeof(T), new_count * sizeof(T)));
            delete[] m_buffer;
            m_count = new_count;
            m_buffer = new_buffer;
            m_size = m_size <= m_count ? m_size : m_count;
            return true;
        }
        bool realloc()
        {
            return realloc(2 * m_count);
        }
        bool trim()
        {
            return realloc(m_size);
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
        bool operator++()
        {
            return realloc();
        }
        bool operator--()
        {
            return trim();
        }
        T& operator[](uint32_t index) const
        {
            return at(index);
        }
    };
}

#endif /* KLIST_HPP */