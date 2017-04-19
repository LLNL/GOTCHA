/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free
Software Foundation) version 2.1 dated February 1999.  This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the terms and conditions of the GNU Lesser General Public License
for more details.  You should have received a copy of the GNU Lesser General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#if !defined(LIBC_WRAPPERS_H_)
#define LIBC_WRAPPERS_H_

/*!
 ******************************************************************************
 *
 * \fn void* gotcha_malloc(size_t size);
 *
 * \brief A gotcha wrapper around malloc to avoid libc dependencies
 *
 * \param size The number of bytes to allocate
 *
 ******************************************************************************
 */
void* gotcha_malloc(size_t size);

/*!
 ******************************************************************************
 *
 * \fn void* gotcha_realloc(void* buffer, size_t size);
 *
 * \brief A gotcha wrapper around malloc to avoid libc dependencies
 *
 * \param buffer The buffer to 
 * \param size The number of bytes to allocate
 *
 ******************************************************************************
 */
void *gotcha_realloc(void* buffer, size_t size);
/*!
 ******************************************************************************
 *
 * \fn void gotcha_free(void* free_me);
 *
 * \brief A gotcha wrapper around free to avoid libc dependencies
 *
 * \param free_me A pointer to the pointer we wish to free
 *
 ******************************************************************************
 */
void gotcha_free(void* free_me);

/*!
 ******************************************************************************
 *
 * \fn void gotcha_memcpy(void* dest, void* src, size_t size);
 *
 * \brief A gotcha wrapper around memcpy to avoid libc dependencies
 *
 * \param dest Where memory should be copied to
 * \param src  Where memory should be copied from
 * \param size How many bytes to copy
 *
 ******************************************************************************
 */
void gotcha_memcpy(void* dest, void* src, size_t size);

/*!
 ******************************************************************************
 *
 * \fn int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
 *
 * \brief A gotcha wrapper around strmcmp to avoid libc dependencies
 *
 * \param in_one The first string to be checked
 * \param in_two The second string to be checked
 * \param max_length The maximum number of characters to compare
 *
 ******************************************************************************
 */

int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
/*!
 ******************************************************************************
 *
 * \fn char* gotcha_strstr(char* searchIn, char* searchFor);
 *
 * \brief A gotcha wrapper around strstr to avoid libc dependencies
 *
 * \param searchIn  A string to search in
 * \param searchFor A substring to search for
 *
 ******************************************************************************
 */
char* gotcha_strstr(char* searchIn, char* searchFor);

/*!
 ******************************************************************************
 *
 * \fn void gotcha_assert(int assertion);
 *
 * \brief A gotcha wrapper around assert to avoid libc dependencies
 *
 * \param assertion Something the programmer says should not equal false
 *
 ******************************************************************************
 */
void gotcha_assert(int assertion);

/*!
 ******************************************************************************
 *
 * \fn int gotcha_strcmp(const char* in_one, const char* in_two);
 *
 * \brief A gotcha wrapper around strcmp to avoid libc dependencies
 *
 * \param in_one The first string to compare
 * \param in_two The second string to compare
 *
 ******************************************************************************
 */
int gotcha_strcmp(const char* in_one, const char* in_two);

char *gotcha_getenv(const char *env);
pid_t gotcha_getpid();
pid_t gotcha_gettid();

#endif
