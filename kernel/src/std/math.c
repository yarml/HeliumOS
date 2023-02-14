#include <math.h>

intmax_t powi(intmax_t base, uintmax_t power)
{
  intmax_t result = 1;
  while(1)
  {
    if (power & 1)
      result *= base;
    power >>= 1;
    if (!power)
      break;
    base *= base;
  }
  return result;
}

uintmax_t exp10i(uintmax_t power)
{
  return powi(10, power);
}

uintmax_t abs(intmax_t i)
{
  return i < 0 ? -i : i;
}
