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

  for (lib = _r_debug.r_map; lib != 0; lib = lib->l_next) {
    if (is_vdso(lib)) {
      continue;
    }
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
        result = lookup_gnu_hash_symbol(binding_iter->name, symtab, strtab,
                                        (struct gnu_hash_header *) gnu_hash);
      }
      if (elf_hash && result == -1) {
        result = lookup_elf_hash_symbol(binding_iter->name, symtab, strtab,
                                        (ElfW(Word) *)elf_hash);
      }

      if (result == -1) {
        not_found++;
        continue;
      }
      if (! GOTCHA_CHECK_VISIBILITY(symtab[result])) {
         continue;
      }

      *(void **)(binding_iter->function_address_pointer) =
         (void *)(symtab[result].st_value + lib->l_addr);
      found++;
    }
  }
  return 0;
}

int gotcha_wrap_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     struct link_map *lmap, struct gotcha_binding_t *syms,
                     int num_actions) {
  int i = 0;
  for (i = 0; i < num_actions; i++) {
    if (gotcha_strcmp(name, syms[i].name) == 0) {
      (*((void **)(lmap->l_addr + offset))) = syms[i].wrapper_pointer;
    }
  }
  return 0;
}

enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, int num_actions, char* tool_name){
  int i;
  enum gotcha_error_t ret_code;
  struct link_map *lib_iter;

  debug_init();

  gotcha_prepare_symbols(bindings, num_actions);

  for (lib_iter = _r_debug.r_map; lib_iter != 0; lib_iter = lib_iter->l_next) {
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings, num_actions);
  }

  ret_code = GOTCHA_SUCCESS;
  for(i = 0; i<num_actions;i++){
    if(bindings[i].function_address_pointer==0){
      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
    }
  }

  return ret_code;
}
