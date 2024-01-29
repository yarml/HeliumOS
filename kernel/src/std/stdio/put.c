#include <stdio.h>

int putchar(int c) {
  return printf("%c", c);
}

int puts(char const *s) {
  return printf("%s", s);
}
