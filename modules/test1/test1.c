#include <stdio.h>

int a;

int sum(char const *str)
{
  return 3 + a;
}


int module_init()
{
  a = 2;
  int s = sum("aaa");
  printd("Hi??\n");
  return s + 5;
}
