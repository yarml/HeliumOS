#include <capi_core.hpp>
#include <capi/types.hpp>
#include <capi/consts.hpp>

// Temporary debug functions
extern "C" void dbg_out(capi::word,capi::byte);

void dbg_char(char c)
{
    // Bochs e9 hack
    dbg_out(0xE9, c);
}
void dbg_str(char const* str)
{
    for(size_t i = 0; str[i] != 0; ++i)
        dbg_char(str[i]);
}
void dbg_uint(int val)
{
    constexpr char NUMBERS[] = "0123456789";
    static utils::array<char, 32> buf;

    int i = 30;

    for(;val > 10; --i, val /= 10)
        buf[i] = NUMBERS[val % 10];

    buf[i] = NUMBERS[val];

    dbg_str(buf.data() + i);
}

namespace core
{
    void init()
    {
        dbg_str("Hello, World\n");
        dbg_uint(5);
        dbg_char('\n');
    }
}
