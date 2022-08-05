#ifndef CSTD_STRING_H
#define CSTD_STRING_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

typedef int(*fpt_chr_predicate     )(int     );
typedef int(*fpt_chr_predicate_args)(int, ...);

// str* functions

/** 
 * \brief Returns the length of the string 
 * \param s: The target string
 * \return The length of the string
 **/
size_t strlen(char const* s);

/**
 * \brief Finds the first occurence of a character in a string
 * \param s: The string to look in
 * \param c: The character to look for
 * \return A pointer to the character if found, otherwise NULL
**/
char* strchr(char const* s, int c);

char* strcat(char* to, char const* from);
char* strcpy(char* to, char const* from);

/**
 * \brief Finds the first occurence of a character that verifies a condition
 * \param s: The string to look in
 * \param pred: The condition to verify
 * \return A pointer to the first character c that verifies pred(c), 
 *         unless the null termination is reached, in which case NULL
**/
char* strpred(char const* s, fpt_chr_predicate pred);

// mem* functions

/**
 * \brief Finds the first occurence of the byte \a c in the block \a block of size \a size.
 * \param block
 * \param c
 * \param size
 * \return If found, a pointer to the first occurence of \a, otherwise returns \a NULL.
 */
void* memchr (void const* block, int c, size_t size);

void* memnchr (void const* block, int c, size_t size);

/**
 * \brief Compares the blocks of memory \a b1 and \a b2 of size.
 * \param b1
 * \param b2
 * \param size
 * \return The differecne between the first different bytes of \a b1 and \a b2, 0 if they are qual until \a size.
 */
int memcmp (void const* b1, void const* b2, size_t size);

/**
 * \brief Sets \a size bytes after \a block to \a c.
 * \param block
 * \param c
 * \param size
 * \return \a block
 */
void* memset(void* block, int c, size_t size);
/**
 * \brief Copies \a size bytes from \a from to \a to
 * \param to
 * \param from
 * \param size
 * \return \a to
 */
void* memcpy(void* to, void const* from, size_t size);

/**
 * \brief Moves \a size bytes from \a from to \a to, and those areas can be overlapping
 * \param to
 * \param from
 * \param size
 * \return \a to
 */
void* memmove(void* to, void const* from, size_t size);

/**
 * \brief Calculates the 8-bit sum of \a size signed bytes after \a block
 * \param block
 * \param size
 * \return The 8-bit sum as an  \a int
 */
int memsum(void* block, size_t size);

// * functions

/**
 * \brief Converts the signed integer \a i into a string ending at \a tail - 1.
 * \param i: The number to convert
 * \param base: The base with which \a i is interpreted
 * \param tail: Pointer to where the caller expects there to be the end of the string,
 *              this function will start from there and write the number backwards untils it finishes
 *              (not inclusing tail in the write),
 *              as such, the caller should reserve enough area before \a tail
 * \return A pointer to the new string
 */
char* ntos(intmax_t n, int base, char* tail);

/**
 * \brief Converts the unsigned integer \a i into a string ending at \a tail - 1.
 * \param i: The number to convert
 * \param base: The base with which \a i is interpreted
 * \param tail: Pointer to where the caller expects there to be the end of the string,
 *              this function will start from there and write the number backwards untils it finishes
 *              (not inclusing tail in the write),
 *              as such, the caller should reserve enough area before \a tail
 * \return A pointer to the new string
 */
char* utos(uintmax_t n, int base, char* tail);

/**
 * \brief Converts a string to a signed number
 * \param s: The string to convert
 * \param tail: Where to store the address of the character immediatly following the number, 
 *              if NULL, it is simply ignored
 * \param base
 * \return The number
 */
intmax_t ston(char const* s, char const** tail, int base);

/**
 * \brief Converts a string to an unsugned number
 * \param s: The string to convert
 * \param tail: Where to store the address of the character immediatly following the number, 
 *              if NULL, it is simply ignored
 * \param base
 * \return The number
 */
uintmax_t stou(char const* s, char const** tail, int base);

#endif
