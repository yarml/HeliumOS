#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

intmax_t ston(char const *s, char const **tail, int base) {
  intmax_t res = 0;
  char    *lsd = strpred(s, isndigit) - 1;
  if (tail) *tail = lsd + 1;
  if (s > lsd) return 0;
  for (; s <= lsd; ++s) res += (*s - '0') * powi(base, lsd - s);
  return res;
}

uintmax_t stou(char const *s, char const **tail, int base) {
  return ston(s, tail, base);
}
