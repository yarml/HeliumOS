#ifndef PAGE_LIST_HPP
#define PAGE_LIST_HPP

#include <kutils.hpp>

extern bool kmain_called;

namespace memory
{
    template <typename T>
    class page_list
    {
    private:
        static uint32_t s_id_count;
    private:
        uint32_t m_count; // Allocated pages
        uint32_t m_size; // Number of entries
        uint32_t m_pstart; // Start page
        uint32_t m_id;
        uint32_t m_called_flag;
# define CALLED_MAGIC (0xDEADEAD)
    public:
        page_list()
            : m_count(1),
              m_size(0),
              m_pstart(kallocvp(1))
        {
            if(m_called_flag != CALLED_MAGIC)
            {
                dbg << "A Page list constructor: " << m_id << "->" << s_id_count + 1 << " size of type " << sizeof(T) << '\n';
                m_id = s_id_count++;
                m_called_flag = CALLED_MAGIC;
            }
            else
                dbg << "Called tha page list: " << m_id << " constructor twice\n";
        }
        ~page_list()
        {
            if(m_pstart != INVALID_PAGE)
            {
                memory::freevp(m_pstart, m_count);
                m_pstart = INVALID_PAGE;
            }
        }
    public:
        bool add(const T& t)
        {
            if(m_count * 4096 < sizeof(T) * (m_size + 1))
            {
                if(!realloc(2 * m_count))
                    return false;
            }
            start()[m_size++] = t;
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
            kutils::mem_copy(ptrto(index), ptrto(index + 1), sizeof(T) * (m_size - index - 1));
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
            m_size = m_size * sizeof(T) <= m_count * 4096 ? m_size : (m_count * 4096) / sizeof(T);
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
    template <typename T>
    uint32_t page_list<T>::s_id_count = 0;
}

#endif /* PAGE_LIST_HPP */