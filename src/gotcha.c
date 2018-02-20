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

#include "libc_wrappers.h"
#include "gotcha/gotcha.h"
#include "gotcha/gotcha_types.h"
#include "gotcha_utils.h"
#include "gotcha_auxv.h"
#include "elf_ops.h"
#include "tool.h"

static void writeAddress(void* write, void* value){
  *(void**)write = value;
}

static void** getBindingAddressPointer(struct gotcha_binding_t* in){
  return (void**)in->function_address_pointer;
}

static void setBindingAddressPointer(struct gotcha_binding_t* in, void* value){
  writeAddress(getBindingAddressPointer(in), value);
}

int gotcha_prepare_symbols(binding_t *bindings, int num_names) {
  struct link_map *lib;
  struct gotcha_binding_t *binding_iter;
  signed long result;
  int found = 0, not_found = 0;
  struct gotcha_binding_t *user_bindings = bindings->user_binding->user_binding;
  debug_printf(1, "Looking up exported symbols for %d table entries\n", num_names);
  for (lib = _r_debug.r_map; lib != 0; lib = lib->l_next) {
    if (is_vdso(lib)) {
      debug_printf(2, "Skipping VDSO library at 0x%lx with name %s\n",
                   lib->l_addr, LIB_NAME(lib));
      continue;
    }
    debug_printf(2, "Searching for exported symbols in %s\n", LIB_NAME(lib));
    INIT_DYNAMIC(lib);
    gotcha_assert(gnu_hash || elf_hash);

    int binding_check = 0;
    for (binding_check = 0, binding_iter = user_bindings; binding_check < num_names;
         binding_iter++, binding_check++) {
      if (*getBindingAddressPointer(binding_iter) != 0x0) {
        continue;
      }

      result = -1;
      if (gnu_hash) {
        debug_printf(3, "Checking GNU hash for %s in %s\n",
                     binding_iter->name, LIB_NAME(lib));
        result = lookup_gnu_hash_symbol(binding_iter->name, symtab, strtab,
                                        (struct gnu_hash_header *) gnu_hash);
      }
      if (elf_hash && result == -1) {
        debug_printf(3, "Checking ELF hash for %s in %s\n",
                     binding_iter->name, LIB_NAME(lib));
        result = lookup_elf_hash_symbol(binding_iter->name, symtab, strtab,
                                        (ElfW(Word) *)elf_hash);
      }

      if (result == -1) {
        debug_printf(3, "%s not found in %s\n", 
                     binding_iter->name, LIB_NAME(lib));
        not_found++;
        continue;
      }
      if (! GOTCHA_CHECK_VISIBILITY(symtab[result])) {
         debug_printf(3, "Symbol %s found but not exported in %s\n", 
                      binding_iter->name, LIB_NAME(lib));
         continue;
      }

      debug_printf(2, "Symbol %s found in %s at 0x%lx\n", 
                   binding_iter->name, LIB_NAME(lib),
                   symtab[result].st_value + lib->l_addr);

      setBindingAddressPointer(binding_iter,(void *)(symtab[result].st_value + lib->l_addr)); 
      found++;
    }
  }
  debug_printf(1, "Found %d / %d during exported symbol lookup\n",
               found, num_names);
  return 0;
}
void gotcha_insert_binding(struct internal_binding_t* binding,  void** got_entry){
  struct internal_binding_t* head;
  int hash_result;
  const char* name = binding->user_binding->name;
  int insert_priority = gotcha_get_priority(binding->associated_binding_table->tool->tool_name);
  hash_result = lookup_hashtable(function_hash_table, (void*)name, (void**)&head);
  if(hash_result != 0){
    addto_hashtable(function_hash_table, (void*)name, (void*)binding);
    writeAddress(got_entry, binding->user_binding->wrapper_pointer);
  }
  else{
    int priority = gotcha_get_priority(head->associated_binding_table->tool->tool_name);
    if(priority < insert_priority){ // if I'm the new head of the list of calls
      binding->next_binding = head;
      (*(void**)binding->user_binding->function_address_pointer) = head->user_binding->wrapper_pointer;
      writeAddress(got_entry, binding->user_binding->wrapper_pointer);
      removefrom_hashtable(function_hash_table, (void*)name);
      addto_hashtable(function_hash_table, (void*)name, (void*)binding);
    }
    else{
      printf("Pure insert on %s (%d), head priority %d\n", binding->associated_binding_table->tool->tool_name, gotcha_get_priority(binding->associated_binding_table->tool->tool_name), gotcha_get_priority(head->associated_binding_table->tool->tool_name));
      while((head->next_binding) && (gotcha_get_priority(head->next_binding->associated_binding_table->tool->tool_name) > insert_priority) ){
        head = head->next_binding;
      }
      if(head->user_binding->wrapper_pointer != binding->user_binding->wrapper_pointer){
        if(head->next_binding){
          *(void**)(binding->user_binding->function_address_pointer) = head->next_binding->user_binding->wrapper_pointer;
          binding->next_binding = head->next_binding;
        }
      head->next_binding = binding;
      *(void**)(head->user_binding->function_address_pointer) = binding->user_binding->wrapper_pointer;
      }
    }
  }
}
int gotcha_wrap_impl(ElfW(Sym) * symbol KNOWN_UNUSED, char *name, ElfW(Addr) offset,
                     struct link_map *lmap, binding_t *bindings) {
  int result;
  binding_ref_t *ref;
  struct internal_binding_t *internal_binding;
  result = lookup_hashtable(&bindings->binding_hash, name, (void **) &ref);
  if (result != 0)
     return 0;
  binding_t* bind = (binding_t*)(ref->binding->user_binding + ref->index);
  internal_binding = ((struct internal_binding_t*)(ref->binding->user_binding + ref->index));
  //internal_binding = ((struct gotcha_binding_t*)(ref->binding->user_binding->user_binding + ref->index))->opaque_handle;
  tool_t* tool = internal_binding->associated_binding_table->tool; 
  int tool_priority = gotcha_get_priority(tool->tool_name);
  void* current_address = (*((void **)(lmap->l_addr + offset)));
  struct binding_t* binding_iter;
  gotcha_insert_binding(internal_binding, (void**)(lmap->l_addr + offset));
  //writeAddress((((void **)(lmap->l_addr + offset))), internal_binding->user_binding->wrapper_pointer);
  debug_printf(3, "Remapped call to %s at 0x%lx in %s to wrapper at 0x%p on INDEX\n",
             name, (lmap->l_addr + offset), LIB_NAME(lmap), 
             internal_binding->user_binding->wrapper_pointer);
  return 0;
}
#ifndef MAX
#define MAX(a,b) (a>b?a:b)
#endif

GOTCHA_EXPORT enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* user_bindings, int num_actions, const char* tool_name){

  unsigned int page_size = gotcha_getpagesize();
  int i;
  enum gotcha_error_t ret_code;
  struct link_map *lib_iter;
  tool_t *tool;

  gotcha_init();

  //First we rewrite anything being wrapped to NULL, so that we can recognize unfound entries
   
  for (i = 0; i < num_actions; i++) {
    setBindingAddressPointer(&user_bindings[i], NULL);
  }

  for(lib_iter=_r_debug.r_map;lib_iter;lib_iter=lib_iter->l_next){
    INIT_DYNAMIC(lib_iter);
    if(got){
      size_t protect_size = MAX(rel_count * rel_size, page_size);
      if(protect_size%page_size){
        protect_size += page_size -  ((protect_size) %page_size);
      }
      ElfW(Addr) prot_address = BOUNDARY_BEFORE(got,(ElfW(Addr))page_size);
      int res = gotcha_mprotect((void*)prot_address,protect_size,PROT_READ | PROT_WRITE | PROT_EXEC );
      if(res == -1){ // mprotect returns -1 on an error
        debug_printf(1, "GOTCHA attempted to mark the GOT table as writable and was unable to do so, calls to wrapped functions will likely fail\n");
      }
    }
  } 
  debug_printf(1, "User called gotcha_wrap for tool %s with %d bindings\n",
               tool_name, num_actions);

  if (debug_level >= 3) {
    for (i = 0; i < num_actions; i++) {
       debug_bare_printf(3, "\t%d: %s will map to %p\n", i, user_bindings[i].name,
                         user_bindings[i].wrapper_pointer);
    }
  }

  if (!tool_name)
     tool_name = "[UNSPECIFIED]";
  tool = get_tool(tool_name);
  if (!tool)
     tool = create_tool(tool_name);
  if (!tool) {
     error_printf("Failed to create tool %s\n", tool_name);
     return GOTCHA_INTERNAL;
  }

  binding_t *bindings = add_binding_to_tool(tool, user_bindings, num_actions);
  if (!bindings) {
     error_printf("Failed to create bindings for tool %s\n", tool_name);
     return GOTCHA_INTERNAL;
  }

  gotcha_prepare_symbols(bindings, num_actions);
  int db =0;
  for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
    debug_printf(2, "Looking for wrapped callsites in %s\n", LIB_NAME(lib_iter));
    if(libraryFilterFunc(lib_iter)){
      FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings);
    }
  }

  ret_code = GOTCHA_SUCCESS;
  for(i = 0; i<num_actions;i++){
    if(*getBindingAddressPointer(&user_bindings[i]) ==NULL ){
       debug_printf(1, "Returning GOTCHA_FUNCTION_NOT_FOUND from gotcha_wrap" 
                    "because of entry %d\n", i);
      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
    }
  }
  debug_printf(1, "Returning code %d from gotcha_wrap\n", ret_code);
  return ret_code;
}

enum gotcha_error_t gotcha_configure_int(const char* tool_name, enum gotcha_config_key_t configuration_key , int value){
  tool_t * tool = get_tool(tool_name);
  if(tool==NULL){
    tool = create_tool(tool_name);
  }
  if( configuration_key == GOTCHA_PRIORITY){
    tool->config.priority = value;
  }
  else{
    debug_printf(1, "Invalid property being configured on tool %s\n", tool_name);
    return GOTCHA_INVALID_CONFIGURATION;
  }
  return GOTCHA_SUCCESS;
}

GOTCHA_EXPORT enum gotcha_error_t gotcha_set_priority(const char* tool_name, int value){
  enum gotcha_error_t error_on_set = gotcha_configure_int(tool_name, GOTCHA_PRIORITY, value);
  if(error_on_set != GOTCHA_SUCCESS) {
    debug_printf(1, "Failed to set priority on tool %s\n", tool_name);
    return GOTCHA_INVALID_CONFIGURATION;
  }
  tool_t* tool_to_place = get_tool(tool_name);
  if(!tool_to_place){
     tool_to_place = create_tool(tool_name);
  }
  remove_tool_from_list(tool_to_place);
  reorder_tool(tool_to_place);
  return GOTCHA_SUCCESS;
}
