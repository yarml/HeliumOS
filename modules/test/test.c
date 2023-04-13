/* This module is here for testing purposes */

#include <stdlib.h>
#include <stdio.h>

int c;

void f()
{
  printf("Hello ");
}

void g()
{
  printf("World ");
}

// This should be called from HeliumOS
int module_init()
{
  f();
  g();
  printf("from Kernel module!\n");
  return 42;
}
