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

#if !defined(TOOL_H_)
#define TOOL_H_

#include "gotcha/gotcha.h"
#include "gotcha/gotcha_types.h"
#include "hash.h"

struct tool_t;

#define UNSET_PRIORITY -1

/**
 * A structure representing how a given tool's bindings are configured
 */
struct gotcha_configuration_t {
  int priority;
};

/**
 * The internal structure that matches the external gotcha_binding_t.
 * In addition to the data specified in the gotcha_binding_t, we add:
 * - a hash table mapping function names to binding_ref_t
 * - a linked-list pointer to the next binding table for this tool
 * - a linked-list pointer to the next binding table
 **/
typedef struct binding_t {
   struct tool_t *tool;
   struct gotcha_binding_t *user_binding;
   int user_binding_size;
   hash_table_t binding_hash;
   struct binding_t *next_tool_binding;
   struct binding_t *next_binding;
} binding_t;

/**
 * A structure for representing tools. Once we support stacking multiple
 * tools this will become more important.
 **/
typedef struct tool_t {
   const char *tool_name;
   binding_t *binding;
   struct tool_t *next_tool;
   struct gotcha_configuration_t config;
   hash_table_t child_tools;
   struct tool_t * parent_tool;
} tool_t;

/**
 * The target for hash tables that map from symbol names to 
 * binding_t entries.
 **/
typedef struct binding_ref_t {
   char *symbol_name;
   binding_t *binding;
   int index;
} binding_ref_t;

tool_t *create_tool(const char *tool_name);
tool_t *get_tool(const char *tool_name);
void reorder_tool(tool_t* new_tool);

binding_t *add_binding_to_tool(tool_t *tool, struct gotcha_binding_t *user_binding, int user_binding_size);
binding_t *get_bindings();
binding_t *get_tool_bindings(tool_t *tool);

struct gotcha_configuration_t get_configuration_for_tool(const char* tool_name_in);
struct gotcha_configuration_t get_default_configuration();
enum gotcha_error_t get_configuration_value(const char* tool_name, enum gotcha_config_key_t key, void* data);

#endif
