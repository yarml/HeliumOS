#include <stdio.h>

int a;

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
  a = 2;
  int s = sum("aaa");
  printd("Hi??\n");
  return s;
}
