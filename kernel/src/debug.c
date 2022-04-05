#include <debug.h>
#include <boot_info.h>

void dbg_draw(uint8_t r, uint8_t g, uint8_t b, uint32_t offx, uint32_t offy)
{
    uint32_t color = 0;
    switch(bootboot.fb_type)
    {
#define ORDER(fi, s, t, fo) ((fi << 24) | (s << 16) | (t << 8) | (fo << 0))
        case FB_ARGB:
            color = ORDER(0, r, g, b);
            break;
        case FB_RGBA:
            color = ORDER(r, g, b, 0);
            break;
        case FB_ABGR:
            color = ORDER(0, b, g, r);
            break;
        case FB_BGRA:
            color = ORDER(b, g, r, 0);
            break;
#undef ORDER
    }
    int x, y;
    for(y = offy * 12; y < offy * 12 + 12; ++y) 
        for(x = offx * 12; x < offx * 12 + 12; ++x)
            *((uint32_t*)(&fb + y * bootboot.fb_scanline + x * 4)) = color;
}
