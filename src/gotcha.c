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

#include "gotcha/gotcha.h"
#include "gotcha_utils.h"
#include "elf_ops.h"
#include "tool.h"

int gotcha_prepare_symbols(binding_t *bindings, int num_names) {
  struct link_map *lib;
  struct gotcha_binding_t *binding_iter;
  signed long result;
  int found = 0, not_found = 0;
  struct gotcha_binding_t *user_bindings = bindings->user_binding;

  int iter = 0;
  for (iter = 0; iter < num_names; iter++) {
    *(void **)(user_bindings[iter].function_address_pointer) = NULL;
  }

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
      if (*(void **)(binding_iter->function_address_pointer) != 0x0) {
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

      *(void **)(binding_iter->function_address_pointer) =
         (void *)(symtab[result].st_value + lib->l_addr);
      found++;
    }
  }
  debug_printf(1, "Found %d / %d during exported symbol lookup\n",
               found, num_names);
  return 0;
}

int gotcha_wrap_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     struct link_map *lmap, binding_t *bindings,
                     int num_actions) {
  int result;
  binding_ref_t *ref;
  struct gotcha_binding_t *user_binding;
  result = lookup_hashtable(&bindings->binding_hash, name, (void **) &ref);
  if (result != 0)
     return 0;

  user_binding = ref->binding->user_binding + ref->index;
  (*((void **)(lmap->l_addr + offset))) = user_binding->wrapper_pointer;

  debug_printf(3, "Remapped call to %s at 0x%lx in %s to wrapper at 0x%p\n",
               name, (lmap->l_addr + offset), LIB_NAME(lmap), 
               user_binding->wrapper_pointer);

  return 0;
}
size_t get_page_size(void)
{
  size_t n;
  char *p;
  int u;
  for (n = 1; n; n *= 2) {
    p = mmap(0, n * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (p == MAP_FAILED) return -1;
    u = munmap(p + n, n);
    munmap(p, n * 2);
    if (!u) return n;
  }
  return -1;
}
#define MAX(a,b) (a>b?a:b)
enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* user_bindings, int num_actions, char* tool_name){
  int page_size = get_page_size();
  int i;
  enum gotcha_error_t ret_code;
  struct link_map *lib_iter;
  tool_t *tool;
  for(lib_iter=_r_debug.r_map;lib_iter;lib_iter=lib_iter->l_next){
    INIT_DYNAMIC(lib_iter);
    if(got){
      int res = mprotect(BOUNDARY_BEFORE(got,page_size),MAX(rel_count*rel_size,page_size),PROT_WRITE|PROT_READ|PROT_EXEC);
      if(!res){
        debug_printf(1, "GOTCHA attempted to mark the GOT table as writable and was unable to do so, calls to wrapped functions will likely fail\n");
      }
    }
  } 
  debug_init();
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

  for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
    debug_printf(2, "Looking for wrapped callsites in %s\n", LIB_NAME(lib_iter));
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings, num_actions);
  }

  ret_code = GOTCHA_SUCCESS;
  for(i = 0; i<num_actions;i++){
    if(user_bindings[i].function_address_pointer==0){
       debug_printf(1, "Returning GOTCHA_FUNCTION_NOT_FOUND from gotcha_wrap" 
                    "because of entry %d\n", i);
      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
    }
  }

  debug_printf(1, "Returning code %d from gotcha_wrap\n", ret_code);
  return ret_code;
}
