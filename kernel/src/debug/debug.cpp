#include <debug.hpp>
#include <dev/io.hpp>
#include <utils/string.hpp>

namespace debug
{
    void dbg_write(const char* str)
    {
        for(;*str != 0; ++str)
            dev::io::outb(BOCHS_OUT_PORT, *str);
    }
    void dbg_write(const uint32_t n)
    {
        uint32_t len = utils::string::uint_len(n);
        char buf[len + 1];
        utils::string::uint_to_str(n, buf);
        buf[len] = 0;
        dbg_write(buf);
    }
}

