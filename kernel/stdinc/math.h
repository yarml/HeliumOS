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

// cast cast cast cast cast cast cast
#define UNITS(n) (uint64_t) (                ((uint64_t) n)                  )
#define KILOS(n) (uint64_t) ((uint64_t) UNITS((uint64_t) n) / (uint64_t) 1024)
#define MEGS( n) (uint64_t) ((uint64_t) KILOS((uint64_t) n) / (uint64_t) 1024)
#define GIGS( n) (uint64_t) ((uint64_t) MEGS( (uint64_t) n) / (uint64_t) 1024)
#define TERAS(n) (uint64_t) ((uint64_t) GIGS( (uint64_t) n) / (uint64_t) 1024)
#define PETAS(n) (uint64_t) ((uint64_t) TERAS((uint64_t) n) / (uint64_t) 1024)
#define EXAS( n) (uint64_t) ((uint64_t) PETAS((uint64_t) n) / (uint64_t) 1024)

// b should be a power of 2
#define ALIGN_DN(n, b) (((n)          ) & ~((b) - 1))
#define ALIGN_UP(n, b) (((n) + (b) - 1) & ~((b) - 1))

#endif