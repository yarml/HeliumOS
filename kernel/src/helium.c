#include <stdlib.h>
#include <stdio.h>

// called from std/start.c:_start()
int kmain()
{
  // TESTING
  printf("Hello, World!\n");
  printf("Let's try allocating something\n");

  void *t0 = malloc(24);
  void *t1 = malloc(32);
  void *t2 = malloc(64);
  void *t3 = malloc(16);

  printf("WE GOT: %p %p %p %p\n", t0, t1, t2, t3);

  // loading modules, and starting the init process should be done here
  return 0;
}

