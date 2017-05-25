#include "gotcha_dl.h"
#include "tool.h"
#include "libc_wrappers.h"
#include "elf_ops.h"

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
