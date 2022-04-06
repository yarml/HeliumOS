#include <debug.h>
#include <fb.h>

void init()
{
    fb_init();
    fb_wr(": [[[ %#+8x ]]]", 123);
    LOOP;
}
