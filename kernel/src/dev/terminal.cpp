#include <dev/terminal.hpp>
#include <stdint.h>
#include <debug/debug.hpp>
#include <dev/framebuffer.hpp>
#include <dev/keyboard.hpp>

namespace terminal
{
    static uint32_t cursor = 0;
    static bool aw = true; // Allow Write
    static bool uppecase = false;
    void key_event(uint8_t scancode)
    {
        dbg << "Scancode: " << (uint32_t) scancode << '\n';
        switch(scancode)
        {
        case LSHIFT_PRESS:
        case LSHIFT_RELEASE:
        case RSHIFT_PRESS:
        case RSHIFT_RELEASE:
        case CAPS_PRESS:
            uppecase = !uppecase;
            break;
        default:
            if(!(scancode & 0x80))
            {
                if(!uppecase)
                    terminal::write(kbdfr[scancode]);
                else
                    terminal::write(kbdfr_upper[scancode]);
            }
            break;
        }
    }
    void write(char c, framebuffer_color fg,framebuffer_color bg)
    {
        if(c == 0)
            return;
        if(aw)
        {
            switch(c)
            {
            case '\n':
                cursor += 80;
                cursor /= 80;
                cursor *= 80;
                break;
            case '\b':
                framebuffer::put_char(--cursor, ' ', fg, bg);
                break;
            case '\t':
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
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
            write(s[i], fg, bg);
    }
    void allow_write(bool a)
    {
        aw = a;
    }
    char kbdfr[128] =
    {
        0,  27, '&', 0, '"', '\'', '(', '-', 0, '_',	/* 9 */
        0, 0, ')', '=', '\b',	/* Backspace */
        '\t',			/* Tab */
        'a', 'z', 'e', 'r',	/* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',	/* Enter key */
            0,			/* 29   - Control */
        'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm',	/* 39 */
        ' ', '*',   0,		/* Left shift */
        '<', 'w', 'x', 'c', 'v', 'b', 'n',			/* 49 */
        ',', ';', ':', '!',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0	/* All other keys are undefined */
    };
    char kbdfr_upper[128] =
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
        '9', '0', 0, '+', '\b',	/* Backspace */
        '\t',			/* Tab */
        'A', 'Z', 'E', 'R',	/* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, '\n',	/* Enter key */
            0,			/* 29   - Control */
        'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',	/* 39 */
        '%', 0,   0,		/* Left shift */
        '>', 'W', 'X', 'C', 'V', 'B', 'N',			/* 49 */
        '?', '.', '/', ' ',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0	/* All other keys are undefined */
    };
}
