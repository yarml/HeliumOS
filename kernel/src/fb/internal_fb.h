#ifndef HELIUM_INTERNAL_FB_H
#define HELIUM_INTERNAL_FB_H

#include <fb.h>

extern psf_font _binary_kernel__font_psf_start;

#define font (_binary_kernel__font_psf_start)

#define SCALEX (2)
#define SCALEY (2)

#define CHRW (font.width  * SCALEX)
#define CHRH (font.height * SCALEY)

#define FB_WIDTH  (bootboot.fb_width  / CHRW)
#define FB_HEIGHT (bootboot.fb_height / CHRH)

extern uint32_t internal_fb_curs_x, internal_fb_curs_y;

#endif
