#include <math.h>

int powi(int base, int power)
{
    int result = 1;
    for(; power != 0; --power, result *= base);
    return result;
}

int exp10i(int power)
{
    return powi(10, power);
}

int abs(int i)
{
    return i < 0 ? -i : i;
}
