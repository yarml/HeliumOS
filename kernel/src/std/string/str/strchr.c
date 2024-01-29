#include <string.h>

char *strchr(char const *s, int c) {
  size_t len = strlen(s);
  return memchr(s, c, len);
}
