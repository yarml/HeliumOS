#include <string.h>

char *strpred(char const *s, fpt_chr_predicate pred)
{
  for(; !pred(*s); ++s)
    if(*s == 0)
      return 0;
  return (char*) s;
}
