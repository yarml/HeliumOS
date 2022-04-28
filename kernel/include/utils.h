#ifndef HELIUM_UTILS_H
#define HELIUM_UTILS_H

/** 
 * \brief Returns the \a n th bit of \a b
 * \param b
 * \param n
 */
#define BIT(b, n) (((b) >> (n)) & 0x01)

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

// returns index of first set bit, -1 if not bit is set
#define FFS(n) (__builtin_ffsll((uint64_t) n) - 1)

#define BITRANGE(s, e) ((uint64_t)((UINT64_MAX >> (64 - ((e) - (s)))) << (s)))

#endif