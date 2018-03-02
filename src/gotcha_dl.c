#define _GNU_SOURCE
#include "gotcha_dl.h"
#include "tool.h"
#include "libc_wrappers.h"
#include "elf_ops.h"
#include <dlfcn.h>

void* _dl_sym(void* handle, const char* name, void* where);

static void*(*orig_dlopen)(const char* filename, int flags);
static void*(*orig_dlsym)(void* handle, const char* name);
struct rev_iter* get_reverse_tool_iterator(struct binding_t* in){
  struct rev_iter* rev_builder = (struct rev_iter*)malloc(sizeof(struct rev_iter));
  rev_builder->next = NULL;
  struct rev_iter* rever;
  struct binding_t* tool_iter = in;
  for(;tool_iter!=NULL;tool_iter = tool_iter->next_binding){
    rev_builder->data = tool_iter;
    rever =  (struct rev_iter*)malloc(sizeof(struct rev_iter));
    rever->next = rev_builder;
    rev_builder = rever;
  }
  return rev_builder;
}
void free_reverse_iterator(struct rev_iter* free_me){
  struct rev_iter* next;
  while(free_me){
    next = free_me->next;
    free(free_me);
    free_me = next;
  }
}
void* dlopen_wrapper(const char* filename, int flags){
  void* handle = orig_dlopen(filename,flags);
  struct binding_t* tool_iter = get_bindings();
  /**
   * The dlopen'ed file is added to the end of the link map. We only have to overwrite symbols in it, and so we only look there
   */
  onlyFilterLast();
  for(;tool_iter!=NULL;tool_iter = tool_iter->next_binding){
      gotcha_wrap(tool_iter->user_binding->user_binding,tool_iter->user_binding_size,tool_iter->tool->tool_name);
  }
  restoreLibraryFilterFunc();
  return handle;
}
void* dlsym_wrapper(void* handle, const char* symbol_name){
  if(handle == RTLD_NEXT){
    return _dl_sym(RTLD_NEXT, symbol_name ,__builtin_return_address(0));
  }
  struct binding_t* tool_iter = get_bindings();
  // TODO: free this chain
  struct rev_iter* rev = get_reverse_tool_iterator(tool_iter);
  for(;rev!=NULL;rev = rev->next){
    struct binding_t* tool_iter = rev->data;
    if(tool_iter){
      int loop = 0;
      for(loop=0;loop<tool_iter->user_binding_size;loop++){
        if(gotcha_strcmp(tool_iter->user_binding->user_binding[loop].name,symbol_name)==0){
          free_reverse_iterator(rev);
          return tool_iter->user_binding->user_binding[loop].wrapper_pointer;
        }
      }
    }
  }
  free_reverse_iterator(rev);
  return orig_dlsym(handle,symbol_name);
}

void handle_libdl(){
  static struct gotcha_binding_t dl_binds[] = {
    {"dlopen", dlopen_wrapper, &orig_dlopen},
    {"dlsym", dlsym_wrapper, &orig_dlsym}
  };     
  gotcha_wrap(dl_binds, 2, "gotcha");
}
#undef _GNU_SOURCE
