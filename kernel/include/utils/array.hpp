#include <stddef.h>

namespace utils
{
    template <class type, size_t len>
    struct array
    {
        // No constructor for aggregate type
    private:
        typedef type* iterator;
        typedef type const* const_iterator;
    public: // container functions
        // at
        // TODO: add bound checking with proper error handling
        // Keep in mind this error handling should be supported on the architecture level
        constexpr type const& at(size_t idx) const
        {
            return m_data[idx];
        }
        constexpr type& at(size_t idx)
        {
            return m_data[idx];
        }
        // front
        constexpr type const& front() const 
        {
            return m_data[0];
        }
        constexpr type& front()
        {
            return m_data[0];
        }
        //back
        constexpr type const& back() const
        {
            return m_data[len - 1];
        }
        constexpr type& back()
        {
            return m_data[len - 1];
        }
        // data
        constexpr type const* data() const
        {
            return m_data;
        }
        constexpr type* data()
        {
            return m_data;
        }
        // begin
        constexpr iterator begin()
        {
            return m_data;
        }
        constexpr const_iterator begin() const
        {
            return m_data;
        }
        constexpr const_iterator cbegin() const
        {
            return m_data;
        }
        // end
        constexpr iterator end()
        {
            return m_data + len;
        }
        constexpr const_iterator end() const
        {
            return m_data + len;
        }
        constexpr const_iterator cend() const
        {
            return m_data + len;
        }
        // empty
        constexpr bool empty() const
        {
            return len == 0;
        }
        // size
        constexpr size_t size() const
        {
            return len;
        }
        // max_size
        constexpr size_t max_size() const
        {
            return len;
        }
        // fill
        constexpr void fill(type const& value)
        {
            for(type& t : *this)
                t = value;
        }
        // swap
        constexpr void swap(array<type, len>& other)
        {
            for(size_t i = 0; i < len; ++i)
            {
                type tmp = other[i];
                other[i] = at(i);
                at(i) = tmp;
            }
        }
    public: // Operators
        constexpr type const& operator[](size_t idx) const
        {
            return at(idx);
        }
        constexpr type& operator[](size_t idx)
        {
            return at(idx);
        }
        constexpr bool operator==(array<type, len> const& other) const
        {
            for(size_t i = 0; i < len; ++i)
                if(other[i] == at(i))
                    continue;
                else
                    return false;
            return true;
        }
    public: // Data; Kept public for aggregate type
        type m_data[len];
    };
}