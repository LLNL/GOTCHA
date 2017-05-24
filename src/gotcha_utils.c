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

#include "gotcha_utils.h"
#include "tool.h"
#include "libc_wrappers.h"
#include "elf_ops.h"
#include "gotcha/gotcha.h"
#include <stdlib.h>

int debug_print_impl(ElfW(Sym) * symbol, char *name, ElfW(Addr) offset,
                     char *filter)
{
  if (gotcha_strstr(name, filter)) {
     debug_printf(1, "Symbol name: %s, offset %lu, size %lu\n", name, offset,
                  symbol->st_size);
  }
  return 0;
}

int debug_print(struct link_map *libc, char *filter)
{
  FOR_EACH_PLTREL(libc, debug_print_impl, filter);
  return 0;
}

int debug_level;
void debug_init()
{
   static int debug_initialized = 0;

   char *debug_str;
   if (debug_initialized) {
      return;
   }
   debug_initialized = 1;
   
   debug_str = gotcha_getenv(GOTCHA_DEBUG_ENV);
   if (!debug_str) {
      return;
   }

   debug_level = gotcha_atoi(debug_str);
   if (debug_level <= 0)
      debug_level = 1;

   debug_printf(0, "Gotcha debug initialized at level %d\n", debug_level);
}

void*(*orig_dlopen)(const char* filename, int flags);
void*(*orig_dlsym)(void* handle, const char* name);

void* dlopen_wrapper(const char* filename, int flags){
  void* handle = orig_dlopen(filename,flags);
  filterLibrariesByName("libdl");
  struct binding_t* tool_iter = get_bindings();
  for(;tool_iter!=NULL;tool_iter = tool_iter->next_binding){
    /**
     * If you remove this comparison, you will overwrite
     * libdl's dlsym relocation with gotcha's, leading to
     * infinite recursion. This is cool, but inefficient.
     */
    if(gotcha_strcmp(tool_iter->tool->tool_name, "gotcha")!=0){
      gotcha_wrap(tool_iter->user_binding,tool_iter->user_binding_size,tool_iter->tool->tool_name);
    }
  }
  restoreLibraryFilterFunc();
  return handle;
}
void* dlsym_wrapper(void* handle, const char* symbol_name){
  int found = 0;
  struct binding_t* tool_iter = get_bindings();
  for(;tool_iter!=NULL;tool_iter = tool_iter->next_binding){
    struct gotcha_binding_t* user_bindings;
    int loop = 0;
    for(loop=0;loop<tool_iter->user_binding_size;loop++){
      if(gotcha_strcmp(tool_iter->user_binding[loop].name,symbol_name)==0){
        return tool_iter->user_binding[loop].wrapper_pointer;
      }
    }
  }
  return orig_dlsym(handle,symbol_name);
}
void handle_libdl(){
  static struct gotcha_binding_t dl_binds[] = {
    {"dlopen", dlopen_wrapper, &orig_dlopen},
    {"dlsym", dlsym_wrapper, &orig_dlsym}
  };     
  gotcha_wrap(dl_binds, 2, "gotcha");
}


void gotcha_init(){
   static int gotcha_initialized = 0;
   if(gotcha_initialized){
     return;
   }
   gotcha_initialized = 1;
   debug_init();
   handle_libdl();
}

