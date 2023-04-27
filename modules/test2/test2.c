#include <stdio.h>

int a = 3;

int sum(char const *str)
{
  int s = 0;
  while(*str)
  {
    s += *str;
    ++str;
  }
  return s + a;
}

int module_init()
{
  int s = sum("ccc");
  printd("And this is test module 2!\n");
  return s + a;
}