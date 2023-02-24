#include <stdlib.h>

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
  // TODO: I assume there is no integer
  // oveflow happening from the multiplication
  // is it even worth fixing?
  return realloc(ptr, nmemb * size);
}