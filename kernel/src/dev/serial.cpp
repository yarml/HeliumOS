#include <dev/serial.hpp>
#include <dev/io.hpp>
#include <dev/framebuffer.hpp>
#include <debug/debug.hpp>

serial com1(SERIAL_COM1_BASE);

serial::serial(uint16_t base)
{
    dbg << "It was a serial constructor\n";
    m_base = base;
}

void serial::configure_baud_rate(uint16_t divisor)
{
    configure_line(serial_line_config::ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(m_base), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(m_base), (divisor     ) & 0x00FF);
}
void serial::configure_line(uint8_t config)
{
    outb(SERIAL_LINE_COMMAND_PORT(m_base), config);
}

void serial::configure_buffer(uint8_t config)
{
    outb(SERIAL_FIFO_COMMAND_PORT(m_base), config);
}

void serial::configure_modem(uint8_t config)
{
    outb(SERIAL_MODEM_COMMAND_PORT(m_base), config);
}

bool serial::query_status(line_status status)
{
    return inb(SERIAL_LINE_STATUS_PORT(m_base)) & (uint8_t) status;
}

void serial::write(uint8_t data)
{
    while(!query_status(line_status::BUF_EMPTY));
    outb(SERIAL_DATA_PORT(m_base), data);
}

void serial::write(const uint8_t* data, uint16_t len)
{
    for(uint32_t i = 0; i < len; i++)
        write(data[i]);
}
