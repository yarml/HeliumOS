#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <stdint.h>

#define SERIAL_COM1_BASE                0x3F8
#define SERIAL_COM2_BASE                0x2F8
#define SERIAL_COM3_BASE                0x3E8
#define SERIAL_COM4_BASE                0x2E8

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

enum serial_line_config
{
    ENABLE_DLAB          = 0b10000000,
    ENABLE_BREAK_CONTROL = 0b01000000,
    PARITY_1             = 0b00001000,
    PARITY_2             = 0b00010000,
    PARITY_3             = 0b00011000,
    PARITY_4             = 0b00100000,
    PARITY_5             = 0b00101000,
    PARITY_6             = 0b00110000,
    PARITY_7             = 0b00111000,
    STOP_BIT             = 0b00000100,
    DATA_LEN_1           = 0b00000001,
    DATA_LEN_2           = 0b00000010,
    DATA_LEN_3           = 0b00000011
};

enum serial_fifo_config
{
    LVL_1  = 0b01000000,
    LVL_2  = 0b10000000,
    LVL_3  = 0b11000000,
    BUFS64 = 0b00100000,
    DMA    = 0b00001000,
    CLTR   = 0b00000100,
    CLRC   = 0b00000010,
    ENABLE = 0b00000001
};

enum serial_modem_config
{
    AUTOFLOW_CTL_ENABLE = 0b00100000,
    LOOPBACK            = 0b00010000,
    AUX2                = 0b00001000,
    AUX1                = 0b00000100,
    RTS                 = 0b00000010,
    DTR                 = 0b00000001
};

enum line_status
{
    IMP_ERR   = 0b10000000,
    TR_IDLE   = 0b01000000,
    BUF_EMPTY = 0b00100000,
    BREAK     = 0b00010000,
    FRAM_ERR  = 0b00001000,
    PRTY_ERR  = 0b00000100,
    OVRN_ERR  = 0b00000010,
    DATA_RDY  = 0b00000001
};

class serial
{
private:
    uint16_t m_base;
public:
    serial(uint16_t base);
public:
    void configure_baud_rate(uint16_t divisor);
    void configure_line(uint8_t config       );
    void configure_buffer(uint8_t config     );
    void configure_modem(uint8_t config      );

    bool query_status(line_status status);

    void write(      uint8_t  data              );
    void write(const uint8_t* data, uint16_t len);
};

extern serial com1;

#endif /* SERIAL_HPP */