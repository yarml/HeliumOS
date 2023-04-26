#include <stdio.h>

int sum(char const *str)
{
  int s = 0;
  while(*str)
  {
    s += *str;
    ++str;
  }
  return s;
}

int module_init()
{
  int s = sum("aaa");
  return s;
}