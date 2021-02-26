#include <dev/keyboard.hpp>

#include <stdint.h>
#include <dev/io.hpp>
#include <debug/debug.hpp>

namespace keyboard
{

    void keyboard_int()
    {
        uint8_t scancode = inb(KEYBOARD_DATA);
        dbg << "scan code: " << (uint32_t) scancode << '\n';
    }
}
