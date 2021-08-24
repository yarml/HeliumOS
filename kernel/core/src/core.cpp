#include <capi_core.hpp>
#include <capi/types.hpp>
#include <capi/consts.hpp>

// Temporary debug functions
void dbg_char(capi::io_interface* io, char c)
{
    // bochs e9 hack
    io->write_byte(0xE9, c);
}
void dbg_str(capi::io_interface* io, char const* str)
{
    for(size_t i = 0; str[i] != 0; ++i)
        dbg_char(io, str[i]);
}
void dbg_uint(capi::io_interface* io, size_t val)
{
    constexpr char NUMBERS[] = "0123456789";
    static utils::array<char, 32> buf;

    if(val == 0)
    {
        dbg_char(io, '0');
        return;
    }

    int i = 30;

    for(;val != 0; --i, val /= 10)
        buf[i] = NUMBERS[val % 10];

    dbg_str(io, buf.data() + i + 1);
}

namespace core
{
    void init(capi::architecture* arch)
    {
        dbg_str(arch->get_io_interface(), "Hello, World\n");
        dbg_uint(arch->get_io_interface(), 159698);
        dbg_char(arch->get_io_interface(), '\n');
    }
}
