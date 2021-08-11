#include <utils/string.hpp>

namespace utils::string
{
    uint32_t uint_len(uint32_t n)
    {
        if(n == 0)
            return 1;
        int counter = 0;
        for(; n > 0; n /= 10, counter++);
        return counter;
    }
    void uint_to_str(uint32_t n, char* buf)
    {
        if(n < 10)
        {
            buf[0] = '0' + n;
            buf[1] = 0;
            return;
        }

        int i = 0;
        for(;n > 0; buf[i] = '0' + n % 10, i++, n /= 10);
        buf[i] = 0;
        str_reverse(buf);
    }
    void str_reverse(char* str)
    {
        uint32_t len = str_len(str);
        for(uint32_t i = 0; i < len / 2; i++)
        {
            char tmp = str[i];
            str[i] = str[len - i - 1];
            str[len - i - 1] = tmp;
        }
    }
    uint32_t str_len(char* str)
    {
        int size = 0;
        for(; *str != 0; str++, size++);
        return size;
    }
}
