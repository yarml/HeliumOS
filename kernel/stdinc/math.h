#ifndef CSTD_MATH_H
#define CSTD_MATH_H

/**
 * \brief Calculates \a to the power of \a power.
 * \param base
 * \param power
 * \return \a base to the power of \a power.
 */
int powi(int base, int power);

/**
 * \brief Calculates 10 to the power of \a power.
 * \param power
 * \return 10 the power of \a power.
 */
int exp10i(int power);

/**
 * \brief Calculates the absolute value of \a i
 * \param i
 */
int abs(int i);

#define UNITS(n) (     (n)       )
#define KILOS(n) (UNITS(n) / 1024)
#define MEGS( n) (KILOS(n) / 1024)
#define GIGS( n) (MEGS( n) / 1024)
#define TERAS(n) (GIGS( n) / 1024)
#define PETAS(n) (TERAS(n) / 1024)
#define EXAS( n) (PETAS(n) / 1024)

#endif