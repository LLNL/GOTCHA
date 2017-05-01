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

#include "tool.h"
#include "libc_wrappers.h"
#include "gotcha_utils.h"

static tool_t *tools = NULL;
static binding_t *all_bindings = NULL;

tool_t *create_tool(char *tool_name)
{
   tool_t *newtool = (tool_t *) gotcha_malloc(sizeof(tool_t));
   if (!newtool) {
      error_printf("Failed to malloc tool %s\n", tool_name);
      return NULL;
   }
   newtool->tool_name = tool_name;
   newtool->binding = NULL;
   newtool->next_tool = tools;
   tools = newtool;
   debug_printf(1, "Created new tool %s\n", tool_name);
   return newtool;
}

tool_t *get_tool(char *tool_name)
{
   tool_t *t;
   for (t = tools; t; t = t->next_tool) {
      if (strcmp(tool_name, t->tool_name) == 0) {
         return t;
      }
   }
   return NULL;
}

binding_t *add_binding_to_tool(tool_t *tool, struct gotcha_binding_t *user_binding, int user_binding_size)
{
   binding_t *newbinding;
   binding_ref_t *ref_table;
   int result, i;

   newbinding = (binding_t *) gotcha_malloc(sizeof(binding_t));
   newbinding->tool = tool;
   newbinding->user_binding = user_binding;
   newbinding->user_binding_size = user_binding_size;
   result = create_hashtable(&newbinding->binding_hash, user_binding_size * 2, 
                             (hash_func_t) strhash, (hash_cmp_t) gotcha_strcmp);
   if (result != 0) {
      error_printf("Could not create hash table for %s\n", tool->tool_name);
      goto error; // error is a label which frees allocated resources and returns NULL
   }

   ref_table = (binding_ref_t *) gotcha_malloc(sizeof(binding_ref_t) * user_binding_size);
   for (i = 0; i < user_binding_size; i++) {
      ref_table[i].symbol_name = user_binding[i].name;
      ref_table[i].binding = newbinding;
      ref_table[i].index = i;
      result = addto_hashtable(&newbinding->binding_hash, ref_table[i].symbol_name, ref_table+i);
      if (result != 0) {
         error_printf("Could not add hash entry for %s to table for tool %s\n", 
                      ref_table[i].symbol_name, tool->tool_name);
         goto error; // error is a label which frees allocated resources and returns NULL
      }
   }

   newbinding->next_tool_binding = tool->binding;
   tool->binding = newbinding;

   newbinding->next_binding = all_bindings;
   all_bindings = newbinding;

   debug_printf(2, "Created new binding table of size %d for tool %s\n", user_binding_size, tool->tool_name);
   return newbinding;

  error:
   if (newbinding)
      gotcha_free(newbinding);
   if (ref_table)
      gotcha_free(ref_table);
   return NULL;
}

binding_t *get_bindings()
{
   return all_bindings;
}

binding_t *get_tool_bindings(tool_t *tool)
{
   return tool->binding;
}
