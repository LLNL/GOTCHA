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
   void **target = getBindingAddressPointer(in);
   debug_printf(3, "Updating binding address pointer at %p to %p\n", target, value);
   writeAddress(target, value);
}

static int prepare_symbol(struct internal_binding_t *binding)
{
   int result;
   struct link_map *lib;
   struct gotcha_binding_t *user_binding = binding->user_binding;

   debug_printf(2, "Looking up exported symbols for %s\n", user_binding->name);
   for (lib = _r_debug.r_map; lib != 0; lib = lib->l_next) {
      if (is_vdso(lib)) {
         debug_printf(2, "Skipping VDSO library at 0x%lx with name %s\n",
                      lib->l_addr, LIB_NAME(lib));
         continue;
      }
      debug_printf(2, "Searching for exported symbols in %s\n", LIB_NAME(lib));
      INIT_DYNAMIC(lib);

      if (!gnu_hash && !elf_hash) {
         debug_printf(3, "Library %s does not export or import symbols\n", LIB_NAME(lib));
         continue;
      }
      result = -1;
      if (gnu_hash) {
         debug_printf(3, "Checking GNU hash for %s in %s\n",
                      user_binding->name, LIB_NAME(lib));
         result = lookup_gnu_hash_symbol(user_binding->name, symtab, strtab,
                                         (struct gnu_hash_header *) gnu_hash);
      }
      if (elf_hash && result == -1) {
         debug_printf(3, "Checking ELF hash for %s in %s\n",
                      user_binding->name, LIB_NAME(lib));
         result = lookup_elf_hash_symbol(user_binding->name, symtab, strtab,
                                         (ElfW(Word) *)elf_hash);
      }
      if (result == -1) {
         debug_printf(3, "%s not found in %s\n",
                      user_binding->name, LIB_NAME(lib));
         continue;
      }
      if (! GOTCHA_CHECK_VISIBILITY(symtab[result])) {
         debug_printf(3, "Symbol %s found but not exported in %s\n", 
                      user_binding->name, LIB_NAME(lib));
         continue;
      }

      debug_printf(2, "Symbol %s found in %s at 0x%lx\n", 
                   user_binding->name, LIB_NAME(lib),
                   symtab[result].st_value + lib->l_addr);
      setBindingAddressPointer(user_binding,(void *)(symtab[result].st_value + lib->l_addr));
      return 0;
   }
   debug_printf(1, "WARNING: Symbol %s was found in program\n",
                user_binding->name);
   return -1;
}

static void insert_at_head(struct internal_binding_t *binding, struct internal_binding_t *head)
{
   binding->next_binding = head;
   (*(void**)binding->user_binding->function_address_pointer) = head->user_binding->wrapper_pointer;
   removefrom_hashtable(function_hash_table, (void*) binding->user_binding->name);
   addto_hashtable(function_hash_table, (void*)binding->user_binding->name, (void*)binding);
}

static void insert_after_pos(struct internal_binding_t *binding, struct internal_binding_t *pos)
{
   setBindingAddressPointer(binding->user_binding, *((void **) pos->user_binding->function_address_pointer));
   setBindingAddressPointer(pos->user_binding, binding->user_binding->wrapper_pointer);
   binding->next_binding = pos->next_binding;
   pos->next_binding = binding;
}

#define RWO_NOCHANGE 0
#define RWO_NEED_LOOKUP (1 << 0)
#define RWO_NEED_BINDING (1 << 1)
static int rewrite_wrapper_orders(struct internal_binding_t* binding)
{
  const char* name = binding->user_binding->name;
  int insert_priority = get_priority(binding->associated_binding_table->tool);

  debug_printf(2, "gotcha_rewrite_wrapper_orders for binding %s in tool %s of priority %d\n",
               name, binding->associated_binding_table->tool->tool_name, insert_priority);

  struct internal_binding_t* head;
  int hash_result;
  hash_result = lookup_hashtable(function_hash_table, (void*)name, (void**)&head);
  if(hash_result != 0) {
    debug_printf(2, "Adding new entry for %s to hash table\n", name);
    addto_hashtable(function_hash_table, (void *) name, (void *) binding);
    return (RWO_NEED_LOOKUP | RWO_NEED_BINDING);
  }

  int head_priority = get_priority(head->associated_binding_table->tool);
    if (head_priority < insert_priority) {
     debug_printf(2, "New binding priority %d is greater than head priority %d, adding to head\n",
                   insert_priority, head_priority);
     insert_at_head(binding, head);
     return RWO_NEED_BINDING;
  }

  struct internal_binding_t* cur;
  for (cur = head; cur->next_binding; cur = cur->next_binding) {
     int next_priority = get_priority(cur->next_binding->associated_binding_table->tool);
     debug_printf(3, "Comparing binding for new insertion %d to binding for tool %s at %d\n",
                   insert_priority, cur->next_binding->associated_binding_table->tool->tool_name,
                   next_priority);
     if (next_priority < insert_priority) {
        break;
     }
     if (cur->user_binding->wrapper_pointer == binding->user_binding->wrapper_pointer) {
        debug_printf(3, "Tool is already inserted.  Skipping binding rewrite\n");
        return RWO_NOCHANGE;
     }
  }
  debug_printf(2, "Inserting binding after tool %s\n", cur->associated_binding_table->tool->tool_name);
  insert_after_pos(binding, cur);
  return RWO_NOCHANGE;
}

static int update_lib_bindings(ElfW(Sym) * symbol KNOWN_UNUSED, char *name, ElfW(Addr) offset,
                               struct link_map *lmap, hash_table_t *lookuptable)
{
  int result;
  struct internal_binding_t *internal_binding;
  void **got_address;

  result = lookup_hashtable(lookuptable, name, (void **) &internal_binding);
  if (result != 0)
     return 0;
  got_address = (void**) (lmap->l_addr + offset);
  writeAddress(got_address, internal_binding->user_binding->wrapper_pointer);
  debug_printf(3, "Remapped call to %s at 0x%lx in %s to wrapper at 0x%p on INDEX\n",
             name, (lmap->l_addr + offset), LIB_NAME(lmap),
             internal_binding->user_binding->wrapper_pointer);
  return 0;
}

#ifndef MAX
#define MAX(a,b) (a>b?a:b)
#endif

static int mark_got_writable(struct link_map *lib)
{
   static unsigned int page_size = 0;
   INIT_DYNAMIC(lib);
   if (!got)
      return 0;

   if (!page_size)
      page_size = gotcha_getpagesize();

   size_t protect_size = MAX(rel_count * rel_size, page_size);
   if(protect_size % page_size){
      protect_size += page_size -  ((protect_size) %page_size);
   }
   ElfW(Addr) prot_address = BOUNDARY_BEFORE(got,(ElfW(Addr))page_size);
   debug_printf(3, "Setting library %s GOT table from %p to +%lu to writeable\n",
                LIB_NAME(lib), (void *) prot_address, protect_size);
   int res = gotcha_mprotect((void*)prot_address,protect_size,PROT_READ | PROT_WRITE | PROT_EXEC );
   if(res == -1){ // mprotect returns -1 on an error
      error_printf("GOTCHA attempted to mark the GOT table as writable and was unable to do so,"
                   "calls to wrapped functions may likely fail\n");
   }

   return 0;
}

static void mark_gots_writable()
{
  struct link_map *lib_iter;
  debug_printf(2, "Setting all GOT tables to be writable\n");
  for (lib_iter = _r_debug.r_map; lib_iter; lib_iter = lib_iter->l_next){
     mark_got_writable(lib_iter);
  }
}

GOTCHA_EXPORT enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* user_bindings, int num_actions, const char* tool_name)
{
  int i, not_found = 0, new_bindings_count = 0;
  struct link_map *lib_iter;
  tool_t *tool;
  hash_table_t new_bindings;

  gotcha_init();

  debug_printf(1, "User called gotcha_wrap for tool %s with %d bindings\n",
               tool_name, num_actions);
  if (debug_level >= 3) {
    for (i = 0; i < num_actions; i++) {
       debug_bare_printf(3, "\t%d: %s will map to %p\n", i, user_bindings[i].name,
                         user_bindings[i].wrapper_pointer);
    }
  }
  debug_printf(3, "Initializing %d user binding entries to NULL\n", num_actions);
  for (i = 0; i < num_actions; i++) {
    setBindingAddressPointer(&user_bindings[i], NULL);
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

  debug_printf(2, "Creating internal binding data structures and adding binding to tool\n");
  binding_t *bindings = add_binding_to_tool(tool, user_bindings, num_actions);
  if (!bindings) {
     error_printf("Failed to create bindings for tool %s\n", tool_name);
     return GOTCHA_INTERNAL;
  }

  debug_printf(2, "Processing %d bindings\n", num_actions);
  for (i = 0; i < num_actions; i++) {
     struct internal_binding_t *binding = bindings->internal_bindings + i;

     int result = rewrite_wrapper_orders(binding);
     if (result & RWO_NEED_LOOKUP) {
        debug_printf(2, "Symbol %s needs lookup operation\n", binding->user_binding->name);
        gotcha_assert(*((void **) binding->user_binding->function_address_pointer) == NULL);
        int presult = prepare_symbol(binding);
        if (presult == -1) {
           not_found++;
        }
     }
     if (result & RWO_NEED_BINDING) {
        debug_printf(2, "Symbol %s needs binding from application\n", binding->user_binding->name);
        if (!new_bindings_count) {
           create_hashtable(&new_bindings, num_actions*2, (hash_func_t) strhash, (hash_cmp_t) gotcha_strcmp);
        }
        addto_hashtable(&new_bindings, (void *) binding->user_binding->name, (void *) binding);
        new_bindings_count++;
     }
  }
  
  if (new_bindings_count) {
     mark_gots_writable();
     debug_printf(2, "Searching callsites for %d bindings\n", new_bindings_count);
     for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
        if(!libraryFilterFunc(lib_iter)) {
           debug_printf(3, "Skipping library %s due to libraryFilterFunc\n", LIB_NAME(lib_iter));
           continue;
        }
        FOR_EACH_PLTREL(lib_iter, update_lib_bindings, lib_iter, &new_bindings);
     }
     destroy_hashtable(&new_bindings);
  }

  if (not_found) {
     debug_printf(1, "Could not find bindings for %d / %d functions\n", not_found, num_actions);
     return GOTCHA_FUNCTION_NOT_FOUND;
  }
  debug_printf(1, "Gotcha wrap completed successfully\n");
  return GOTCHA_SUCCESS;
}

static enum gotcha_error_t gotcha_configure_int(const char* tool_name, enum gotcha_config_key_t configuration_key , int value){
  tool_t * tool = get_tool(tool_name);
  if(tool==NULL){
    tool = create_tool(tool_name);
  }
  if( configuration_key == GOTCHA_PRIORITY){
    tool->config.priority = value;
  }
  else{
    error_printf("Invalid property being configured on tool %s\n", tool_name);
    return GOTCHA_INTERNAL;
  }
  return GOTCHA_SUCCESS;
}

GOTCHA_EXPORT enum gotcha_error_t gotcha_set_priority(const char* tool_name, int value){
  gotcha_init();
  debug_printf(1, "User called gotcha_set_priority(%s, %d)\n", tool_name, value);
  enum gotcha_error_t error_on_set = gotcha_configure_int(tool_name, GOTCHA_PRIORITY, value);
  if(error_on_set != GOTCHA_SUCCESS) {
    return error_on_set;
  }
  tool_t* tool_to_place = get_tool(tool_name);
  if(!tool_to_place){
     tool_to_place = create_tool(tool_name);
  }
  remove_tool_from_list(tool_to_place);
  reorder_tool(tool_to_place);
  return GOTCHA_SUCCESS;
}

GOTCHA_EXPORT enum gotcha_error_t gotcha_get_priority(const char* tool_name, int *priority){
  gotcha_init();
  return get_configuration_value(tool_name, GOTCHA_PRIORITY, priority);
}

