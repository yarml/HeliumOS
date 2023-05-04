#include <boot_info.h>

extern int a;

int module_init()
{
  return bootboot.magic[0] + a;
}