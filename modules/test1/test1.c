#include <stdio.h>

int module_init()
{
  char const *test_str = "aaa";
  char const *a = test_str;
  int sum = 0;
  while(*a)
  {
    sum += *a;
    ++a;
  }
  return sum;
}