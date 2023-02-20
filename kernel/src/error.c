#include <string.h>
#include <error.h>
#include <stdio.h>
#include <sys.h>

void error_general(char *prefix, char *details)
{
  fprintf(stderr, "[%s] %s\n", prefix, details);
  stop();
}

void error_out_of_memory(char *details)
{
  error_general("Out of memory", details);
}

void error_inv_state(char *details)
{
  error_general("Invalid state", details);
}
