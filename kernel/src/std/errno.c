#include <errno.h>

static int s_errno = 0;

int *__get_errno()
{
  return &s_errno;
}
