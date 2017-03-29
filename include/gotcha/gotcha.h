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
#include "gotcha_utils.h"
#include "gotcha_types.h"

/*!
 ******************************************************************************
 * \def GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...)
 * \param name     The name of the function you want to get a pointer to
 * \param ret_type The return type of the function you want a pointer to
 * \param ...      A comma separated list of the types of the parameters
 * 								   to the function you're getting a pointer to
 ******************************************************************************
 */

#define GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...) ret_type(*name)(__VA_ARGS__); \

/*!
 ******************************************************************************
 *
 * \fn struct gotcha_binding_t* gotcha_prepare_symbols(char** symbol_names, int num_names);
 *
 * \brief Given a list of function names, create the gotcha structure used to
 *				wrap functions
 *
 * \param symbol_names The names of the symbols to be wrapped
 * \param num_names 	 The number of symbol names in symbol_names
 *
 ******************************************************************************
 */
struct gotcha_binding_t* gotcha_prepare_symbols(char** symbol_names, int num_names);

/*!
 ******************************************************************************
 *
 * \fn int gotcha_wrap(struct gotcha_binding_t* bindings, void** wrappers, void*** originals, int num_actions);
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
int gotcha_wrap(struct gotcha_binding_t* bindings, void** wrappers, void*** originals, int num_actions);
#endif
