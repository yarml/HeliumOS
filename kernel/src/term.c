#include <stdio.h>
#include <term.h>

void term_setfg(int r, int g, int b)
{
  printf("\efg %u %u %u\e", r, g, b);
}

void term_setbg(int r, int g, int b)
{
  printf("\ebg %u %u %u\e", r, g, b);
}
