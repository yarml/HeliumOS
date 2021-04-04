#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <dev/framebuffer.hpp>

namespace terminal
{
    void write(char c, uint8_t scancode = 0,
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void write(char* s, 
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void allow_write(bool a);
}

#endif /* TERMINAL_HPP */