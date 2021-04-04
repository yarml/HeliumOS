#include <dev/terminal.hpp>
#include <stdint.h>

#include <dev/framebuffer.hpp>

namespace terminal
{
    static uint32_t cursor = 0;
    static bool aw = true;
    void write(char c, uint8_t scancode, framebuffer_color fg,framebuffer_color bg)
    {
        if(aw)
        {
            switch(c)
            {
            case 0:
                switch(scancode)
                {
                
                }
                break;
            case '\n':
                
                cursor /= 80;
                cursor += 80;
                break;
            case '\b':
                framebuffer::put_char(--cursor, ' ', fg, bg);
                break;
            default:
                framebuffer::put_char(cursor++, c, fg, bg);
                break;
            }
            if(cursor >= 80 * 25)
            {
                cursor = 0;
                framebuffer::clear();
            }
            framebuffer::set_curs(cursor);

        }
        
    }
    void write(char* s, framebuffer_color fg,framebuffer_color bg)
    {
        if(!aw)
            return;
        for(uint32_t i = 0; s[i] != 0; i++)
            write(s[i], 0, fg, bg);
    }
    void allow_write(bool a)
    {
        aw = a;
    }

}
