#include <stddef.h>
#include <string.h>

int strcmp(char const *s1, char const *s2) {
  if (s1 == s2) {
    return 0;
  }

  size_t s1len = strlen(s1);
  size_t s2len = strlen(s2);

  if (s1len == s2len) {
    return memcmp(s1, s2, s1len);
  }

  return s1len < s2len ? -1 : 1;
}