#ifndef PIT_HPP
#define PIT_HPP

#include <stdint.h>

#define PIT(n)  (0x40 + n)
#define PIT_CTL (0x43    )

#define PIT_FREQ 1193180

namespace pit
{
    enum ctl
    {
        BIN         = 0b00000000,
        BCD         = 0b00000001,
        INT_ON_TERM = 0b00000000,
        HARD_RETRIG = 0b00000010,
        RATE_GEN    = 0b00000100,
        SQUARE_GEN  = 0b00000110,
        SOFT_TRIG   = 0b00001000,
        HARD_TRIG   = 0b00001010,
        RATE_GEN2   = 0b00001100,
        SQUARE_GEN2 = 0b00001110,
        LATCH       = 0b00000000,
        LOW         = 0b00010000,
        HIGH        = 0b00100000,
        LOW_HIGH    = 0b00110000,
        CHANNEL0    = 0b00000000,
        CHANNEL1    = 0b01000000,
        CHANNEL2    = 0b10000000,
        CHANNEL3    = 0b11000000
    };
    void init(uint32_t freq);
    void pit_int();
    void wait_ticks(uint32_t ticks);
    void wait(uint32_t sec);
}


#endif /* PIT_HPP */