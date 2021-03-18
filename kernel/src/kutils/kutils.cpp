#include <kutils.hpp>
#include <string.h>

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
        size_t len = strlen(str);
        for(uint32_t i = 0; i < len / 2; i++)
        {
            char tmp = str[i];
            str[i] = str[len - i - 1];
            str[len - i - 1] = tmp;
        }
    }
}
