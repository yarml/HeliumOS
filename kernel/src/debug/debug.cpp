#include <debug/debug.hpp>
#include <stddef.h>
#include <dev/framebuffer.hpp>
#include <kutils.hpp>

debug dbg(com1);

debug::debug(serial& serial)
{ 
    mp_serial = &serial;
    serial.configure_baud_rate(2                                                  );
    serial.configure_line(serial_line_config::DATA_LEN_3                          );
    serial.configure_buffer(serial_fifo_config::LVL_3 | serial_fifo_config::BUFS64
                        | serial_fifo_config::CLTR  | serial_fifo_config::CLRC 
                        | serial_fifo_config::ENABLE                            );
    serial.configure_modem(0x03);
    dbg << "It was a debug constructor\n";
}

void debug::print(char c)
{
    mp_serial->write(c);
    DEBUG_TERM(c);
}

void debug::print(const char* msg)
{
    size_t len = kutils::string_len(msg);
    mp_serial->write((const uint8_t*)msg, len);
    for(; *msg != 0; msg++)
        DEBUG_TERM(*msg);
}
void debug::print(uint32_t n)
{
    uint32_t n_len = kutils::uint32_length(n);
    char str_n[n_len + 1];
    kutils::uint32_to_string_dec(n, str_n);
    str_n[n_len] = 0;
    print(str_n);
}
void debug::print(void* ptr)
{
    // TODO: print ptr as hex
    print((uint32_t) ptr);
}

debug& debug::operator<<(char c)
{
    print(c);
    return *this;
}

debug& debug::operator<<(const char* msg)
{
    print(msg);
    return *this;
}
debug& debug::operator<<(uint32_t n)
{
    print(n);
    return *this;
}
debug& debug::operator<<(void* ptr)
{
    print(ptr);
    return *this;
}

debug& debug::operator<<(bool b)
{
    print(b ? "true" : "false");
    return *this;
}
