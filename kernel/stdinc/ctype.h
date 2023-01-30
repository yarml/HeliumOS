#ifndef CSTD_CTYPE_H
#define CSTD_CTYPE_H

/**
 * \brief Checks if the character \a c is a valid decimal digit.
 * \param c
 * \return 1 if the character is a valid decimal digit, 0 otherwise.
 */
int isdigit(int c);

/**
 * \brief Checks if the character \a c is not a valid decimal digit.
 * \param c
 * \return 1 if the character is not a valid decimal digit, 0 otherwise.
 */
int isndigit(int c);

/**
 * \brief Checks if the character \a c is a valid digit of base \a base.
 * \param c
 * \return 1 if the character is a valid digit of base \a base, 0 otherwise.
 */
int isbdigit(int c, int base);

/**
 * \brief Checks if the character \a c is a not valid digit of base \a base.
 * \param c
 * \return 1 if the character is not a valid digit of base \a base, 0 otherwise.
 */
int isnbdigit(int c, int base);

/**
 * \brief Checks if the character \a c is an upper case character.
 * \param c
 * \return 1 if the character is an upper case character, 0 otherwise.
 */
int isupper(int c);
/**
 * \brief Checks if the character \a c is not an upper case character.
 * \param c
 * \return 1 if the character is not an upper case character, 0 otherwise.
 */
int isnupper(int c);

/**
 * \brief Checks if the character \a c is a lower case character.
 * \param c
 * \return 1 if the character is a lower case character, 0 otherwise.
 */
int islower(int c);
/**
 * \brief Checks if the character \a c is not a lower case character.
 * \param c
 * \return 1 if the character is not a lower case character, 0 otherwise.
 */
int isnlower(int c);

/**
 * \brief Checks if the character \a c is an alphabetic symbol.
 * \param c
 * \return 1 if the character is an alphabetic symbol, 0 otherwise.
 */
int isalpha(int c);
/**
 * \brief Checks if the character \a c is not an alphabetic symbol.
 * \param c
 * \return 1 if the character is not an alphabetic symbol, 0 otherwise.
 */
int isnalpha(int c);

/**
 * \brief Checks if the character \a c is a decimal digit or an
 *        alphabetic symbol.
 * \param c
 * \return 1 if the character is a decimal digit or an alphabetic symbol.
 *         0 otherwise.
 */
int isalnum(int c);
/**
 * \brief Checks if the character \a c is not a decimal digit or an
 *        alphabetic symbol.
 * \param c
 * \return 1 if the character is not a decimal digit or an alphabetic symbol.
 *         0 otherwise.
 */
int isnalnum(int c);


#endif