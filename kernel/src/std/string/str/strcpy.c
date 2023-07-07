#include <string.h>

char *strcpy(char *to, char const *from) {
  return memcpy(to, from, strlen(from) + 1);
}
