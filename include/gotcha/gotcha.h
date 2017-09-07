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
 * \file gotcha.h
 *
 * \brief   Header file containing the external gotcha interface
 *
 *          The intended use pattern is as follows
 *
 *					TODO ON-INTERFACE-SOLID: document the interface 
 *                                   usage
 *
 ******************************************************************************
 */
#ifndef GOTCHA_H
#define GOTCHA_H

#include "gotcha/gotcha_types.h"

#if defined(__cplusplus) 
extern "C" {
#endif

/*!
 ******************************************************************************
 * \def GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...)
 * \brief Makes a function pointer with a given name, return type, and
 *        parameters
 * \param name     The name of the function you want to get a pointer to
 * \param ret_type The return type of the function you want a pointer to
 * \param ...      A comma separated list of the types of the parameters
 * 								 to the function you're getting a pointer to
 ******************************************************************************
 */

#define GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...) ret_type(*name)(__VA_ARGS__)

#define GOTCHA_EXPORT __attribute__((__visibility__("default")))

/*!
 ******************************************************************************
 *
 * \fn enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, 
 *                                     void** wrappers, void*** originals, 
 *                                     int num_actions);
 *
 * \brief Makes GOTCHA wrap the functions picked in gotcha_prepare_symbols
 *
 * \param bindings    A list of bindings to wrap
 * \param num_actions The number of items in the bindings table
 * \param tool_name   A name you use to represent your tool when
 *                    stacking multiple tools (currently unused).
 *
 ******************************************************************************
 */

GOTCHA_EXPORT enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, int num_actions, const char* tool_name);

/*!
 ******************************************************************************
 *
 * \fn enum void* gotcha_get_wrapped_function(wrapper_id_t wrappee)
 *
 * \brief Returns the underlying function for use in Gotcha wrappers
 *
 * \param wrappee The index of the underlying function the user desires
 *
 ******************************************************************************
 */

GOTCHA_EXPORT void* gotcha_get_wrapped_function(wrapper_id_t wrappee); 

#if defined(__cplusplus) 
}
#endif


#endif
