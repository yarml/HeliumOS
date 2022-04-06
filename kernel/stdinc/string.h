#ifndef HELIUM_STRING_H
#define HELIUM_STRING_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

typedef int(*chr_predicate     )(int         );
typedef int(*chr_predicate_args)(int, va_list);

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


/**
 * \brief Finds the first occurence of a character that verifies a condition
 * \param s: The string to look in
 * \param pred: The condition to verify
 * \return A pointer to the first character c that verifies pred(c), 
 *         unless the null termination is reached, in which case NULL
**/
char* strpred(char const* s, chr_predicate pred);

/** Like strpred, except the predicate function also takes a va_list for additional arguments */
char* strapred(char const* s, chr_predicate_args pred);

/**
 * \brief Converts the signed integer \a i into a string null terminated at \a null.
 * \param i: The number to convert
 * \param base: The base with which \a i is interpreted
 * \param null: Pointer to where the caller expects there to be the null termination of the string,
 *              this function will start from there and write the number backwards untils it finishes,
 *              as such, the caller should reserve enough area before \a null
 * \return A pointer to the new string
 */
char* ntos(intmax_t n, int base, char* null);

/**
 * \brief Converts the unsigned integer \a i into a string null terminated at \a null.
 * \param i: The number to convert
 * \param base: The base with which \a i is interpreted
 * \param null: Pointer to where the caller expects there to be the null termination of the string,
 *              this function will start from there and write the number backwards untils it finishes,
 *              as such, the caller should reserve enough area before \a null
 * \return A pointer to the new string
 */
char* utos(uintmax_t n, int base, char* null);

#endif
