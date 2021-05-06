#include <dev/terminal.hpp>
#include <stdint.h>
#include <debug/debug.hpp>
#include <dev/framebuffer.hpp>
#include <dev/keyboard.hpp>
#include <sys/cmds.hpp>

namespace terminal
{
    static uint32_t cursor = 0;
    static bool aw = true; // Allow Write
    static bool uppecase = false;
    static char current_command[COMMAND_BUFFER];
    static int16_t cc_cursor = 0; // current command cursor

    void prompt()
    {
        write(" > ", false, framebuffer_color::RED);
    }
    static void newline()
    {
        cursor += 80;
        cursor /= 80;
        cursor *= 80;
        if(cursor >= 80 * 25)
        {
            cursor = 0;
            framebuffer::clear();
        }
    }
    static void send_cmd()
    {
        sys::execute(current_command);
        current_command[0] = 0;
        cc_cursor = 0;
        prompt();
    }
    void init()
    {
        prompt();
    }
    void reset(bool _prompt)
    {
        framebuffer::clear();
        cursor = 0;
        if(_prompt)
            prompt();
    }
    void key_event(uint8_t scancode)
    {
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
                    terminal::write(kbdus[scancode], true);
                else
                    terminal::write(kbdus_upper[scancode], true);
            }
            break;
        }
    }

    void write(char c, bool add_to_cmd, framebuffer_color fg,framebuffer_color bg)
    {
        if(c == 0)
            return;
        if(aw)
        {
            switch(c)
            {
            case '\r':
                newline();
                break;
            case '\n':
                newline();
                send_cmd();
                break;
            case '\b':
                if(cc_cursor == 0)
                    return;
                if(current_command[cc_cursor - 1] == '\t')
                {
                    framebuffer::put_char(--cursor, ' ', fg, bg);
                    framebuffer::put_char(--cursor, ' ', fg, bg);
                    framebuffer::put_char(--cursor, ' ', fg, bg);
                    framebuffer::put_char(--cursor, ' ', fg, bg);
                }
                else
                {
                    framebuffer::put_char(--cursor, ' ', fg, bg);
                }
                if(add_to_cmd)
                    current_command[--cc_cursor] = 0;
                break;
            case '\t':
                if(add_to_cmd)
                {
                    if(cc_cursor >= COMMAND_BUFFER - 2)
                        return;
                    current_command[cc_cursor++] = '\t';
                    current_command[cc_cursor + 1] = 0;
                }
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
                framebuffer::put_char(cursor++, ' ', fg, bg);
                break;
            default:
                if(add_to_cmd)
                {
                    if(cc_cursor >= COMMAND_BUFFER - 2)
                        return;
                    current_command[cc_cursor++] = c;
                    current_command[cc_cursor + 1] = 0;
                }
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
    void write(const char* s, bool add_to_cmd, framebuffer_color fg,framebuffer_color bg)
    {
        if(!aw)
            return;
        for(uint32_t i = 0; s[i] != 0; i++)
            write(s[i], add_to_cmd, fg, bg);
    }
    void allow_write(bool a)
    {
        aw = a;
    }
    char kbdus[128] =
    {
        0, '`', '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
        '9', '0', '-', '=', '\b',	/* Backspace */
        '\t',			/* Tab */
        'q', 'w', 'e', 'r',	/* 19 */
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
            0,			/* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
        '\'', '\\',   0,		/* Left shift */
        ' ', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
        'm', ',', '.', '/',   0,				/* Right shift */
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
    char kbdus_upper[128] =
    {
        0, '~', '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
        '(', ')', '_', '+', '\b',	/* Backspace */
        '\t',			/* Tab */
        'Q', 'W', 'E', 'R',	/* 19 */
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\r',	/* Enter key */
            0,			/* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
        '"', '|',   0,		/* Left shift */
        ' ', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
        'M', '<', '>', '?',   0,				/* Right shift */
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
