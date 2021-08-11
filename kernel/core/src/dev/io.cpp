#include <dev/io.hpp>

extern "C" uint8_t  a_inb( uint16_t port               );
extern "C" uint16_t a_inw( uint16_t port               );
extern "C" void     a_outb(uint16_t port, uint8_t data );
extern "C" void     a_outw(uint16_t port, uint16_t data);

namespace dev::io
{
    uint8_t inb(uint16_t port)
    {
        return a_inb(port);
    }
    uint16_t inw(uint16_t port)
    {
        return a_inw(port);
    }

    void outb(uint16_t port, uint8_t data)
    {
        a_outb(port, data);
    }
    void outw(uint16_t port, uint16_t data)
    {
        a_outw(port, data);
    }
}
