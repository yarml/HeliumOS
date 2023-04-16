#include <stdio.h>

static void setfg(int r, int g, int b)
{
  printf("\efg %u %u %u\e", r, g, b);
}

static void prompt(char *user, char *pwd, int super)
{
  setfg(255, 255, 0);
  fputs(user, stdout);
  setfg(255, 0, 0);
  printf(" [");
  setfg(0, 255, 255);
  fputs(pwd, stdout);
  setfg(255, 0, 0);
  printf("] ");
  setfg(255, 255, 0);
  printf(super ? "# " : "$ ");
  setfg(255, 255, 255);
}

int kmain()
{
  prompt("kernel", "initrd://sys", 1);
  return 0;
}
