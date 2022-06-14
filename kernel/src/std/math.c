#include <math.h>

intmax_t powi(intmax_t base, intmax_t power)
{
    intmax_t result = 1;
    for (;;)
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

intmax_t exp10i(intmax_t power)
{
    return powi(10, power);
}

intmax_t abs(intmax_t i)
{
    return i < 0 ? -i : i;
}
