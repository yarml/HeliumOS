#include <string.h>

char* strcat(char* to, char const* from) {
  strcpy(to + strlen(to), from);
  return to;
}