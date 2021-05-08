#include <kutils/kstring.hpp>


namespace kutils
{
    kstring::kstring()
        : m_string()
    {
    }
    kstring::kstring(const kstring& str)
        : m_string()
    {
        m_string.realloc(str.length() + 1);
        mem_copy(m_string.buffer(), str.m_string.buffer(), str.length() + 1);
    }
    kstring::kstring(const char* str)
        : m_string()
    {
        uint32_t len = string_len(str);
        m_string.realloc(len + 1);
        mem_copy(m_string.buffer(), str, len + 1);
    }
    char* kstring::c_str() const
    {
        return m_string.buffer();
    }

    uint32_t kstring::length() const
    {
        return m_string.size() - 1;
    }
}
