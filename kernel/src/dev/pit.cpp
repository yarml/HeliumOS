#include <dev/pit.hpp>
#include <dev/io.hpp>
#include <debug/debug.hpp>

namespace pit
{
    static uint32_t s_freq  = PIT_FREQ;
    static uint32_t s_ticks = 0       ;
    void init(uint32_t freq)
    {
        s_freq = freq;
        uint32_t divisor = PIT_FREQ / freq;

        outb(PIT_CTL, ctl::CHANNEL0 | ctl::LOW_HIGH | ctl::SQUARE_GEN | ctl::BIN); 

        uint8_t l = (divisor   ) & 0xFF;
        uint8_t h = (divisor>>8) & 0xFF;

        outb(PIT(0), l);
        outb(PIT(0), h);
    }
    void pit_int()
    {
        s_ticks++;
    }
    void wait_ticks(uint32_t ticks)
    {
        uint32_t start_tick = s_ticks;
        while(s_ticks - start_tick != ticks);
    }
    void wait(uint32_t sec)
    {
        wait_ticks(sec * s_freq);
    }
    void wait_milli(uint32_t milli)
    {
        wait_ticks(milli * (s_freq / 1000));
    }
}
