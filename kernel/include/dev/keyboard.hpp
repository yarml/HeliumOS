#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <stdint.h>

#define KEYBOARD_DATA   (0x60)
#define KEYBOARD_CTL    (0x61)

#define CAPS_PRESS      (58)
#define CAPS_RELEASE    (186)
#define LSHIFT_PRESS    (42)
#define LSHIFT_RELEASE  (170)
#define RSHIFT_PRESS    (0x36)
#define RSHIFT_RELEASE  (0xB6)

namespace keyboard
{
    void keyboard_int();
}

#endif /* KEYBOARD_HPP */