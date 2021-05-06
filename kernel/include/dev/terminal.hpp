#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <dev/framebuffer.hpp>

#define COMMAND_BUFFER (256)

namespace terminal
{
    void init();
    void key_event(uint8_t scancode);
    void write(char c, bool add_to_cmd = false,
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void write(const char* s, bool add_to_cmd = false,
                framebuffer_color fg = framebuffer_color::WHITE,
                framebuffer_color bg = framebuffer_color::BLACK);
    void reset(bool prompt = true);
    void allow_write(bool a);
    extern char kbdus[128];
    extern char kbdus_upper[128];
}

#endif /* TERMINAL_HPP */