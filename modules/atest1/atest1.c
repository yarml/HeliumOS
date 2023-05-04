#include <boot_info.h>

int module_init()
{
  return bootboot.magic[0];
}