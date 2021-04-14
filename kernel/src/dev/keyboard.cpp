#include <dev/keyboard.hpp>

#include <stdint.h>
#include <dev/io.hpp>
#include <debug/debug.hpp>
#include <dev/framebuffer.hpp>
#include <dev/terminal.hpp>

namespace keyboard
{
    void keyboard_int()
    {
        uint8_t scancode = inb(KEYBOARD_DATA);
        terminal::key_event(scancode);
    }
}
