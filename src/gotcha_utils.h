/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free Software
Foundation) version 2.1 dated February 1999.  This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even the IMPLIED
WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms
and conditions of the GNU Lesser General Public License for more details.  You should
have received a copy of the GNU Lesser General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*!
 ******************************************************************************
 *
 * \file gotcha_utils.h
 *
 * \brief   Header file containing the internal gotcha mechanisms
 *          for manipulating the running process to redirect calls
 *
 ******************************************************************************
 */
#ifndef GOTCHA_UTILS_H
#define GOTCHA_UTILS_H
#include <sys/mman.h>
#include "gotcha/gotcha_types.h"
// TODO: remove these includes
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// END TODO
#include <elf.h>
#include <link.h>
#include <string.h>


#define GOTCHA_DEBUG_ENV "GOTCHA_DEBUG"
extern int debug_level;
extern FILE *debug_io;
void debug_init();

#define debug_bare_printf(lvl, format, ...)       \
   do {                                           \
     if (debug_level >= lvl) {                    \
       fprintf(debug_io, format, ## __VA_ARGS__); \
     }                                            \
   } while (0);

#define SHORT_FILE__ ((strrchr(__FILE__, '/') ? : __FILE__ - 1) + 1)

#define debug_printf(lvl, format, ...)               \
   do {                                              \
     if (debug_level >= lvl) {                       \
       fprintf(debug_io, "[%d/%d][%s:%u] - " format, \
               gotcha_gettid(), gotcha_getpid(),     \
               SHORT_FILE__, __LINE__,               \
               ## __VA_ARGS__);                      \
     }                                               \
   } while (0);

#define error_printf(format, ...)                          \
do {                                                       \
     if (debug_level) {                                    \
       fprintf(debug_io, "ERROR [%d/%d][%s:%u] - " format, \
               gotcha_gettid(), gotcha_getpid(),           \
               SHORT_FILE__, __LINE__,                     \
               ## __VA_ARGS__);                            \
     }                                                     \
   } while (0);

#define LIB_NAME(X) (!X->l_name ? "[NULL]" : (!*X->l_name ? "[EMPTY]" : X->l_name))

/*!
 ******************************************************************************
 * \def R_SYM(X)
 * \brief Returns an ELF symbol which is correct for the current architecture
 * \param X The value you wish to cast to Symbol type
 ******************************************************************************
 */
#if __WORDSIZE == 64
#define R_SYM(X) ELF64_R_SYM(X)
#else
#define R_SYM(X) ELF32_R_SYM(X)
#endif

/*!
 ******************************************************************************
 * \def BOUNDARY_BEFORE(ptr,pagesize)
 * \brief Returns the address on page boundary before the given pointer
 * \param ptr The address you wish to get the page boundary before
 * \param pagesize The page size you wish to align to
 ******************************************************************************
 */
#define BOUNDARY_BEFORE(ptr, pagesize) \
  (void*)(((ElfW(Addr))ptr) & (-pagesize))


/*!
 ******************************************************************************
 *
 * \fn void gotcha_prepare_symbols(struct gotcha_binding_t* bindings, int num_names)
 *
 * \brief Given a list of function names, create the gotcha structure used to
 *				wrap functions
 *
 * \param bindings     The GOTCHA wrap actions
 * \param num_names 	 The number of symbol names in symbol_names
 *
 ******************************************************************************
 */
int gotcha_prepare_symbols(struct gotcha_binding_t* bindings, int num_names);

#endif
