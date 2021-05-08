#ifndef KSTRING_HPP
#define KSTRING_HPP

#include <kutils/klist.hpp>

namespace kutils
{
    class kstring
    {
    private:
        klist<char> m_string;
    public:
        kstring();
        kstring(const kstring& str);
        kstring(const char* str);
    public:
        uint32_t length() const;
        char* c_str() const;
    };
}

#endif /* KSTRING_HPP */