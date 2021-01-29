#include <debug/debug.hpp>
#include <stddef.h>
#include <string.h>
#include <devices/framebuffer.hpp>

serial com1;

void init_dbg()
{
    com1.configure_baud_rate(2                                                  );
    com1.configure_line(serial_line_config::DATA_LEN_3                          );
    com1.configure_buffer(serial_fifo_config::LVL_3 | serial_fifo_config::BUFS64
                        | serial_fifo_config::CLTR  | serial_fifo_config::CLRC 
                        | serial_fifo_config::ENABLE                            );
    com1.configure_modem(0x03);
}

void debug_msg(const char* msg)
{
    size_t len = strlen(msg);
    com1.write((const uint8_t*)msg, len);
    for(; *msg != 0; msg++)
        DEBUG_TERM(*msg);
}

void debug_msgln(const char* msg)
{
    debug_msg(msg);
    com1.write('\n');
    DEBUG_TERM('\n');
}
