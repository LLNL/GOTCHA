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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
 *					TODO ON-INTERFACE-SOLID: document the
 *interface usage
 *
 ******************************************************************************
 */
#ifndef GOTCHA_H
#define GOTCHA_H

#include <link.h>

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
 * 		   to the function you're getting a pointer to
 ******************************************************************************
 */

#define GOTCHA_MAKE_FUNCTION_PTR(name, ret_type, ...) \
  ret_type (*name)(__VA_ARGS__)

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

GOTCHA_EXPORT enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t *bindings,
                                              int num_actions,
                                              const char *tool_name);

/*!
 ******************************************************************************
 *
 * \fn enum gotcha_error_t gotcha_set_priority(const char *tool_name,
 *                                             int value);
 *
 * \brief Set the tool priority, which controls how multiple tools stack
 *        wrappings over the same functions.
 *
 * \param tool_name   The tool name to set the priority of
 * \param priority    The new priority value for the tool.  Lower values
 *                    are called innermost.
 *
 ******************************************************************************
 */
GOTCHA_EXPORT enum gotcha_error_t gotcha_set_priority(const char *tool_name,
                                                      int priority);

/*!
 ******************************************************************************
 *
 * \fn enum gotcha_error_t gotcha_get_priority(const char *tool_name,
 *                                             int *value);
 *
 * \brief Gets the tool priority, which controls how multiple tools stack
 *        wrappings over the same functions.
 *
 * \param tool_name   The tool name to get the priority of
 * \param num_actions Output parameters with the priority for the tool.
 *
 ******************************************************************************
 */
GOTCHA_EXPORT enum gotcha_error_t gotcha_get_priority(const char *tool_name,
                                                      int *priority);

/*!
 ******************************************************************************
 *
 * \fn enum void* gotcha_get_wrappee(gotcha_wrappee_handle_t)
 *
 * \brief Given a GOTCHA wrapper's handle, returns the wrapped function for it
 *to call
 *
 * \param handle The wrappee handle to return the function pointer for
 *
 ******************************************************************************
 */
GOTCHA_EXPORT void *gotcha_get_wrappee(gotcha_wrappee_handle_t handle);

/*!
 * \fn void gotcha_filter_libraries_by_name(const char *nameFilter);
 *
 * \brief This API allows GOTCHA to include only libraries given specified by
 * the user. This could be a partial match of string contains as defined by
 * strstr function in C.
 *
 * \param nameFilter the filter name
 */
GOTCHA_EXPORT void gotcha_filter_libraries_by_name(const char *nameFilter);

/*!
 * \fn void gotcha_only_filter_last();
 *
 * \brief This API allows GOTCHA to include only the last library defined in the
 * linker of the tool.
 */
GOTCHA_EXPORT void gotcha_only_filter_last();

/*!
 * \fn void gotcha_set_library_filter_func(int (*new_func)(struct link_map *));
 *
 * \brief This API allows users to define a function that selected the libraries
 * that user wants to intercept. The function should take struct link_map* as
 * input and return true if it should be wrapped by GOTCHA. TIP: the library
 * name can be accessed by map->l_name.
 *
 * \param new_func has the function signature of int (*new_func)(struct link_map
 * *)
 */
GOTCHA_EXPORT void gotcha_set_library_filter_func(
    int (*new_func)(struct link_map *));

/*!
 * \fn void gotcha_restore_library_filter_func();
 *
 * \brief The default filter of gotcha selects all libraries loaded. This
 * function set the default filter back for GOTCHA.
 */
GOTCHA_EXPORT void gotcha_restore_library_filter_func();

/*!
 * \fn enum gotcha_error_t gotcha_unwrap(const char *tool_name);
 *
 * \brief remove the tool from GOTCHA wrapping to be used as a final routine.
 *
 * \param tool_name, The tool name to remove
 */
GOTCHA_EXPORT enum gotcha_error_t gotcha_unwrap(const char *tool_name);

#if defined(__cplusplus)
}
#endif

#endif
