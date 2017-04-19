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

int gotcha_prepare_symbols(struct gotcha_binding_t *bindings, int num_names) {
  struct link_map *lib;
  struct gotcha_binding_t *binding_iter;
  signed long result;
  int found = 0, not_found = 0;

  int iter = 0;
  for (iter = 0; iter < num_names; iter++) {
    *(void **)(bindings[iter].function_address_pointer) = NULL;
  }

  debug_printf(1, "Looking up exported symbols for %d table entries\n", num_names);
  for (lib = _r_debug.r_map; lib != 0; lib = lib->l_next) {
    if (is_vdso(lib)) {
      debug_printf(2, "Skipping VDSO library at 0x%lx with name %s",
                   lib->l_addr, LIB_NAME(lib));
      continue;
    }
    debug_printf(2, "Searching for exported symbols in %s\n", LIB_NAME(lib));
    INIT_DYNAMIC(lib);
    gotcha_assert(gnu_hash || elf_hash);

    int binding_check = 0;
    for (binding_check = 0, binding_iter = bindings; binding_check < num_names;
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
                     struct link_map *lmap, struct gotcha_binding_t *syms,
                     int num_actions) {
  int i = 0;
  for (i = 0; i < num_actions; i++) {
    if (gotcha_strcmp(name, syms[i].name) == 0) {
      (*((void **)(lmap->l_addr + offset))) = syms[i].wrapper_pointer;
      debug_printf(3, "Remapped call to %s at 0x%lx in %s to wrapper at 0x%p\n",
                   name, (lmap->l_addr + offset), LIB_NAME(lmap), 
                   syms[i].wrapper_pointer);
    }
  }
  return 0;
}

enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, int num_actions, char* tool_name){
  int i;
  enum gotcha_error_t ret_code;
  struct link_map *lib_iter;

  debug_init();
  debug_printf(1, "User called gotcha_wrap for tool %s with %d bindings\n",
               tool_name, num_actions);

  if (debug_level >= 3) {
    for (i = 0; i < num_actions; i++) {
       debug_bare_printf(3, "\t%d: %s will map to %p\n", i, bindings[i].name,
                         bindings[i].wrapper_pointer);
    }
  }

  gotcha_prepare_symbols(bindings, num_actions);

  for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
    debug_printf(2, "Looking for wrapped callsites in %s\n", LIB_NAME(lib_iter));
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings, num_actions);
  }

  ret_code = GOTCHA_SUCCESS;
  for(i = 0; i<num_actions;i++){
    if(bindings[i].function_address_pointer==0){
       debug_printf(1, "Returning GOTCHA_FUNCTION_NOT_FOUND from gotcha_wrap" 
                    "because of entry %d\n", i);
      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
    }
  }

  debug_printf(1, "Returning code %d from gotcha_wrap\n", ret_code);
  return ret_code;
}
