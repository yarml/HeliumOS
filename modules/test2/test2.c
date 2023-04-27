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
  return s;
}