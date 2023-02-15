#include <stdbool.h>
#include <stdint.h>
#include <math.h>

char* ntos(intmax_t n, int base, char *tail){
  bool negative = n < 0;
  do
  {
    --tail;
    if(abs(n % base) < 10)
      *tail = '0' + abs(n % base);
    else
      *tail = 'A' + abs(n % base - 10);
    n /= base;
  } while(n != 0);

  if(negative)
  {
    --tail;
    *tail = '-';
  }
  return tail;
}


char *utos(uintmax_t n, int base, char *tail)
{
  do
  {
    --tail;
    if(n % base < 10)
      *tail = '0' + n % base;
    else
      *tail = 'A' + n % base - 10;
    n /= base;
  } while(n != 0);
  return tail;
}