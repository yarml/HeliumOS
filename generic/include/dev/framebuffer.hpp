#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <stdint.h>

#define FB_WIDTH  80
#define FB_HEIGTH 25

#define FB_DATA_PORT 0x3D5
#define FB_CMD_PORT  0x3D4

#define FB_CMD_HIGH  14
#define FB_CMD_LOW   15

enum framebuffer_color : uint8_t
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GREY, 
    DARK_GREY, 
    LIGTH_BLUE, 
    LIGTH_GREEN,
    LIGHT_CYAN, 
    LIGHT_RED, 
    LIGHT_MAGENTA, 
    LIGHT_BROWN, 
    WHITE
};

struct framebuffer_cell
{
    uint8_t ascii;
    uint8_t fg:4 ;
    uint8_t bg:4 ;
} __attribute__((packed));

namespace framebuffer
{
    extern framebuffer_cell* base;

    void clear(framebuffer_color bg = framebuffer_color::BLACK    );
    void put_char(  uint16_t position   , uint8_t ascii            ,
                    framebuffer_color fg = framebuffer_color::WHITE,
                    framebuffer_color bg = framebuffer_color::BLACK);
    void put_char(  uint8_t x, uint8_t y, uint8_t ascii            ,
                    framebuffer_color fg = framebuffer_color::WHITE,
                    framebuffer_color bg = framebuffer_color::BLACK);
    void put_string(uint8_t x, uint8_t y, const char* str          ,
                    framebuffer_color fg = framebuffer_color::WHITE,
                    framebuffer_color bg = framebuffer_color::BLACK);
    void put_string(uint16_t position    , const char* str         ,
                    framebuffer_color fg = framebuffer_color::WHITE,
                    framebuffer_color bg = framebuffer_color::BLACK);
    void set_curs(uint8_t x, uint8_t y);
    void set_curs( uint16_t position);
}

#endif /* FRAMEBUFFER_HPP */