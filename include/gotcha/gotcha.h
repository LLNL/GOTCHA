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
 * \file gotcha.h
 *
 * \brief   Header file containing the external gotcha interface
 *
 *          The intended use pattern is as follows
 *
 *					TODO ON-INTERFACE-SOLID: document the interface usage
 *
 ******************************************************************************
 */
#ifndef GOTCHA_H
#define GOTCHA_H
#include <stdint.h>
#include <elf.h>
#include <link.h>
#include <gotcha/gotcha_types.h>

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

#define GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...) ret_type(*name)(__VA_ARGS__); \

/*!
 ******************************************************************************
 *
 * \fn enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, void** wrappers, void*** originals, int num_actions);
 *
 * \brief Makes GOTCHA wrap the functions picked in gotcha_prepare_symbols
 *
 * \param bindings    A list of bindings to do wrapping on, should be returned by a call to gotcha_prepare_symbols
 * \param wrappers 	  A list of pointers to wrapper functions the chosen bindings should redirect to
 * \param originals   A list of pointers to original functions to be wrapped from, if you need to call the function
										  you wrapped, pass a reference to a pointer to that function in this list
 * \param num_actions The number of functions being wrapped
 *
 ******************************************************************************
 */
enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, int num_actions);
#endif
