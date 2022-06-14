#ifndef CSTD_MATH_H
#define CSTD_MATH_H

#include <stddef.h>
#include <stdint.h>

/**
 * \brief Calculates \a to the power of \a power.
 * \param base
 * \param power
 * \return \a base to the power of \a power.
 */
intmax_t powi(intmax_t base, intmax_t power);

/**
 * \brief Calculates 10 to the power of \a power.
 * \param power
 * \return 10 the power of \a power.
 */
intmax_t exp10i(intmax_t power);

/**
 * \brief Calculates the absolute value of \a i
 * \param i
 */
intmax_t abs(intmax_t i);

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


#endif