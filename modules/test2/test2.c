#include <stdio.h>
#include <utils.h>

void do_print();

int module_init()
{
  printd("%c\n", g_units_sign[2]);
  do_print();
  return 34;
}