#ifndef HELIUM_UTILS_H
#define HELIUM_UTILS_H

/** 
 * \brief Returns the \a n th bit of \a b
 * \param b
 * \param n
 */
#define BIT(b, n) (((b) >> (n)) & 0x01)

#endif