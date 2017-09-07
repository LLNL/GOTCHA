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
/*!
 ******************************************************************************
 *
 * \file gotcha_types.h
 *
 * \brief   Header file containing the internal gotcha types
 *
 ******************************************************************************
 */
#ifndef GOTCHA_TYPES_H
#define GOTCHA_TYPES_H

#if defined(__cplusplus) 
extern "C" {
#endif

typedef void* wrapper_id_t;

/*!
 * The representation of a GOTCHA action
 * as it passes through the pipeline
 */
struct gotcha_binding_t {
  const char* name;                      //!< The name of the function being wrapped
  void* wrapper_pointer;           //!< A pointer to the wrapper function
  wrapper_id_t function_address_pointer;  //!< A pointer to the function being wrapped
};

/*!
 * The representation of an error (or success) of a GOTCHA action
 */
enum gotcha_error_t {
  GOTCHA_SUCCESS = 0,        //!< The call succeeded
  GOTCHA_FUNCTION_NOT_FOUND, //!< The call looked up a function which could not be found
  GOTCHA_INTERNAL            //!< Internal gotcha error
};

#if defined(__cplusplus) 
}
#endif

#endif
