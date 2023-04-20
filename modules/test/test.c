#include <stdio.h>

int module_init()
{
  printd("Hello, World from kernel module!\n");
  return 42;
}