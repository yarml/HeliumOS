#include <kutils.hpp>

namespace kutils
{
    size_t uint32_length(uint32_t num)
    {
        if(num == 0)
            return 1;
        int counter = 0;
        for(; num > 0; num /= 10, counter++);
        return counter;
    }
    void uint32_to_string_dec(uint32_t num, char* buf)
    {
        if(num < 10)
        {
            buf[0] = '0' + num;
            buf[1] = 0;
            return;
        }

        int i = 0;
        for(;num > 0; buf[i] = '0' + num % 10, i++, num /= 10);
        buf[i] = 0;
        string_reverse(buf);
    }
    void string_reverse(char* str)
    {
        size_t len = kutils::string_len(str);
        for(uint32_t i = 0; i < len / 2; i++)
        {
            char tmp = str[i];
            str[i] = str[len - i - 1];
            str[len - i - 1] = tmp;
        }
    }
    uint32_t string_len(const char* str)
    {
        int size = 0;
        for(; *str != 0; str++, size++);
        return size;
    }

    char* string_chr(const char* str, char c)
    {
        uint32_t size = kutils::string_len(str);
        for(uint32_t i = 0; i < size; i++)
            if(str[i] == c)
                return (char*) &(str[i]);
        return NULL;
    }
    void* mem_copy(void* to, const void* from, uint32_t size)
    {
        for(size_t i = 0; i < size; i++)
            ((char*)to)[i] = ((char*)from)[i];
        return to;
    }
    uint32_t char_pos(const char* str, char c)
    {
        uint32_t size = kutils::string_len(str);
        for(uint32_t i = 0; i < size; i++)
            if(str[i] == c)
                return i;
        return -1;
    }
    int32_t string_compare(const char* str1, const char* str2)
    {
        int32_t size1 = kutils::string_len(str1);
        int32_t size2 = kutils::string_len(str2);
        int32_t size = size1 > size2 ? size2 : size1;
        for(int32_t i = 0; i < size; i++)
            if(str1[i] != str2[i])
                return str1[i] - str2[i];
        return size1 - size2;
    }
    char* string_copy(char* to, const char* from)
    {
        kutils::mem_copy(to, from, kutils::string_len(from) + 1);
        return to;
    }
    int32_t mem_compare(const void* p1, const void* p2, uint32_t size)
    {
        for(uint32_t i = 0; i < size; i++)
            if(((uint8_t*)p1)[i] != ((uint8_t*)p2)[i])
                return ((uint8_t*)p1)[i] - ((uint8_t*)p2)[i];
        return 0;
    }
}
