#include <dev/framebuffer.hpp>
#include <dev/io.hpp>

namespace framebuffer
{
    framebuffer_cell* base = (framebuffer_cell*) 0x000B8000;
    
    void put_char(uint16_t position, uint8_t ascii, 
                            framebuffer_color fg, framebuffer_color bg)
    {
        base[position] = framebuffer_cell{.ascii = ascii, .fg = fg, .bg = bg};
    }

    void put_char(uint8_t x, uint8_t y, uint8_t ascii,
                        framebuffer_color fg, framebuffer_color bg  )
    {
        put_char(x + y * FB_WIDTH, ascii, fg, bg);
    }
    void put_string(uint8_t x, uint8_t y, const char* str     ,
                                framebuffer_color fg, framebuffer_color bg)
    {
        put_string(x + y * FB_WIDTH, str, fg, bg);
    }

    void put_string(uint16_t position   , const char* str    ,
                                framebuffer_color fg,framebuffer_color bg)
    {
        for(uint16_t i = 0; str[i] != 0; i++)
            put_char(i + position, str[i], fg, bg);
    }

    void set_curs(uint8_t x, uint8_t y)
    {
        set_curs(x + y * FB_WIDTH);
    }

    void set_curs(uint16_t position)
    {
        outb(FB_CMD_PORT , FB_CMD_HIGH             );
        outb(FB_DATA_PORT, (position >> 8) & 0x00FF);
        outb(FB_CMD_PORT , FB_CMD_LOW              );
        outb(FB_DATA_PORT, position & 0x00FF       );
    }

    void clear(framebuffer_color bg)
    {
        for(uint8_t x = 0; x < FB_WIDTH; x++)
            for(uint8_t y = 0; y < FB_HEIGTH; y++)
                put_char(x + y * FB_WIDTH, ' ', framebuffer_color::WHITE, bg);
    }
}