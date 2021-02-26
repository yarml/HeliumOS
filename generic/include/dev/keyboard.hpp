#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <stdint.h>

#define KEYBOARD_DATA   (0x60)
#define KEYBOARD_CTL    (0x61)

namespace keyboard
{
    void keyboard_int();
}

#endif /* KEYBOARD_HPP */