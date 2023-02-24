#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size)
{
  // TODO: I assume there is no integer
  // oveflow happening from the multiplication
  // is it even worth fixing?
  void *ptr = malloc(nmemb * size);
  if(ptr)
    memset(ptr, 0, nmemb * size);
  return ptr;
}
