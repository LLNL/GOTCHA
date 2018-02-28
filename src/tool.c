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

tool_t* get_tool_list(){
  return tools;
}
int tool_equal(tool_t* t1, tool_t* t2){
  return gotcha_strcmp(t1->tool_name,t2->tool_name);
}
void remove_tool_from_list(struct tool_t* target){
     const char* name = target->tool_name;
     if(!tools){
        return;
     }
     if(!tool_equal(tools,target)){
        tools = tools->next_tool;
        return;
     }
     struct tool_t *cur = tools;
     while( (cur!=NULL) && (cur->next_tool != NULL) && (tool_equal(cur->next_tool,target))){
        cur = cur->next_tool;
     }
     if(!tool_equal(cur->next_tool,target)){
        cur->next_tool = target->next_tool; 
     }
}

void reorder_tool(tool_t* new_tool) {
  const char* name = new_tool->tool_name;
  int new_priority = new_tool->config.priority;
  if(tools==NULL || tools->config.priority >= new_priority ){
     new_tool->next_tool = tools;
     tools = new_tool;
  }
  else{
     struct tool_t *cur = tools;
     while((cur->next_tool != NULL) && cur->next_tool->config.priority < new_priority){
        cur = cur->next_tool;
     }
     new_tool->next_tool = cur->next_tool;
     cur->next_tool = new_tool;
  }
}

tool_t *create_tool(const char *tool_name)
{
   debug_printf(1, "Found no existing tool with name %s\n",tool_name);
   // TODO: ensure free
   tool_t *newtool = (tool_t *) gotcha_malloc(sizeof(tool_t));
   if (!newtool) {
      error_printf("Failed to malloc tool %s\n", tool_name);
      return NULL;
   }
   newtool->tool_name = tool_name;
   newtool->binding = NULL;
   //newtool->next_tool = tools;
   newtool->config = get_default_configuration();
   reorder_tool(newtool);
   newtool->parent_tool = NULL;
   create_hashtable(&newtool->child_tools, 24, 
     (hash_func_t) strhash, (hash_cmp_t) gotcha_strcmp);
   //tools = newtool;
   debug_printf(1, "Created new tool %s\n", tool_name);
   return newtool;
}
tool_t *get_tool(const char *tool_name)
{
   tool_t *t;
   for (t = tools; t; t = t->next_tool) {
      if (gotcha_strcmp(tool_name, t->tool_name) == 0) {
         return t;
      }
   }
   return NULL;
}

binding_t *add_binding_to_tool(tool_t *tool, struct gotcha_binding_t *user_binding, int user_binding_size)
{
   binding_t *newbinding;
   binding_ref_t *ref_table = NULL;
   int result, i;
   newbinding = (binding_t *) gotcha_malloc(sizeof(binding_t));
   newbinding->tool = tool;
   struct internal_binding_t* internal_bindings = (struct internal_binding_t*)gotcha_malloc(sizeof(struct internal_binding_t)*user_binding_size);
   for(i=0;i<user_binding_size;i++){
      internal_bindings[i].is_rewritten = 0;
      internal_bindings[i].user_binding = &user_binding[i];
      user_binding[i].opaque_handle = &internal_bindings[i]; 
      internal_bindings[i].associated_binding_table = newbinding;
   }  
   //newbinding->user_binding = user_binding;
   newbinding->internal_bindings = internal_bindings;
   newbinding->internal_bindings_size = user_binding_size;
   result = create_hashtable(&newbinding->binding_hash, user_binding_size * 2, 
                             (hash_func_t) strhash, (hash_cmp_t) gotcha_strcmp);
   if (result != 0) {
      error_printf("Could not create hash table for %s\n", tool->tool_name);
      goto error; // error is a label which frees allocated resources and returns NULL
   }
   ref_table = (binding_ref_t *) gotcha_malloc(sizeof(binding_ref_t) * user_binding_size);
   for (i = 0; i < user_binding_size; i++) {
      ref_table[i].symbol_name = (char *) user_binding[i].name;
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

/**
 * TODO DO-NOT-MERGE "/" should be a macro of possible separators
 */
struct gotcha_configuration_t get_configuration_for_tool(const char* tool_name_in){
  printf("Creating config for tool %s\n",tool_name_in);
  gotcha_init();
  tool_t* possible_tool = get_tool(tool_name_in);
  if(possible_tool){
    printf("Returning config predefined\n");
    return possible_tool->config;
  }
  else{ // TODO: do-not-merge, no hierarchy
     possible_tool = create_tool(tool_name_in);
     return possible_tool->config;
  }
  char tool_name[512];
  strncpy(tool_name,tool_name_in,512);
  char* string_iter;
  string_iter = strtok((char*)tool_name, "/"); // TODO DO-NOT-MERGE gotcha_strtok
  printf("strtok pointer %p value %s\n",(void*)string_iter,string_iter);
  struct tool_t* tool_iter = get_tool(tool_name);
  if(tool_iter == NULL){
    tool_iter = create_tool(tool_name);
  }
  struct tool_t* lookup_key; 
  char intermediate_name[512]; //TODO DO-NOT-MERGE implement, and is 512 enough?
  strncpy(intermediate_name, string_iter, 512); //TODO DO-NOT-MERGE gotcha_strncpy
  string_iter = strtok(NULL, "/"); // TODO DO-NOT-MERGE gotcha_strtok
  while(string_iter!=NULL){
    int lookup = lookup_hashtable(&tool_iter->child_tools, string_iter,(hash_data_t*) &lookup_key); 
    strncat(intermediate_name, "/", 1); //TODO DO-NOT-MERGE gotcha_strncat
    strncat(intermediate_name, string_iter, 512); //TODO DO-NOT-MERGE gotcha_strncat
    if(lookup==-1){
      char* new_tool_name = (char*)malloc(sizeof(char)*512); //TODO DO-NOT-MERGE implement, and is 512 enough?
      struct tool_t* new_tool = create_tool(new_tool_name);
      strncpy(new_tool_name,intermediate_name,512);
      addto_hashtable(&tool_iter->child_tools, string_iter, new_tool);
      new_tool->parent_tool = tool_iter;
      tool_iter = new_tool;
    }
    else{
      tool_iter = lookup_key;
    }
    string_iter = strtok(NULL, "/"); // TODO DO-NOT-MERGE gotcha_strtok
  }
  return tool_iter->config;
}

struct gotcha_configuration_t get_default_configuration(){
  struct gotcha_configuration_t result;
  result.priority = UNSET_PRIORITY;
  return result;
}
enum gotcha_error_t get_default_configuration_value(enum gotcha_config_key_t key, void* data){
  struct gotcha_configuration_t config = get_default_configuration();
  if(key==GOTCHA_PRIORITY){
    int current_priority = config.priority;
    *((int*)(data)) = config.priority; 
  }
  return GOTCHA_SUCCESS;

}

int gotcha_get_priority(const char* tool_name){
  int return_value;
  get_configuration_value(tool_name, GOTCHA_PRIORITY,&return_value);
  return return_value;
}
enum gotcha_error_t get_configuration_value(const char* tool_name, enum gotcha_config_key_t key, void* location_to_store_result){
  struct tool_t* tool = get_tool(tool_name);
  if(tool==NULL){
    debug_printf(1, "Property being examined for nonexistent tool %s\n", tool_name);
    return GOTCHA_INVALID_CONFIGURATION;
  }
  get_default_configuration_value(key, location_to_store_result);
  int found_valid_value = 0;
  while( (tool!=NULL) && !(found_valid_value) ){
    struct gotcha_configuration_t config = tool->config;
    if(key==GOTCHA_PRIORITY){
      int current_priority = config.priority;
      if(current_priority!=UNSET_PRIORITY){
        *((int*)(location_to_store_result)) = config.priority; 
        found_valid_value = 1;
        return GOTCHA_SUCCESS;
      }
    }
    else{
      debug_printf(1, "Invalid property being configured on tool %s\n", tool_name);
      return GOTCHA_INVALID_CONFIGURATION;
    }
    tool = tool->parent_tool;
  }
  return GOTCHA_SUCCESS;  
}
