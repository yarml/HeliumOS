/* This module is here for testing purposes */

#include <stdlib.h>
#include <stdio.h>

// This should be called from HeliumOS
int module_init()
{
  printf("Hello World from Kernel module!\n");
  return 42;
}
