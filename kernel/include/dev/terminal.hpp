#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <dev/framebuffer.hpp>

namespace terminal
{
    void key_event(uint8_t scancode);
    void write(char c,
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void write(char* s, 
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void allow_write(bool a);
    extern char kbdfr[128];
    extern char kbdfr_upper[128];
}

#endif /* TERMINAL_HPP */