#define _GNU_SOURCE
#include "gotcha_dl.h"
#include "tool.h"
#include "libc_wrappers.h"
#include "elf_ops.h"
#include <dlfcn.h>

void* _dl_sym(void* handle, const char* name, void* where);

static void*(*orig_dlopen)(const char* filename, int flags);
static void*(*orig_dlsym)(void* handle, const char* name);

static struct rev_iter* get_reverse_tool_iterator(struct binding_t* in){
  struct rev_iter* rev_builder = (struct rev_iter*) gotcha_malloc(sizeof(struct rev_iter));
  rev_builder->next = NULL;
  struct rev_iter* rever;
  struct binding_t* tool_iter = in;
  for(;tool_iter!=NULL;tool_iter = tool_iter->next_binding){
    rev_builder->data = tool_iter;
    rever =  (struct rev_iter*) gotcha_malloc(sizeof(struct rev_iter));
    rever->next = rev_builder;
    rev_builder = rever;
  }
  return rev_builder;
}

static void free_reverse_iterator(struct rev_iter* free_me){
  struct rev_iter* next;
  while(free_me){
    next = free_me->next;
    gotcha_free(free_me);
    free_me = next;
  }
}

static int per_binding(hash_key_t key, hash_data_t data, void *opaque)
{
   int result;
   struct internal_binding_t *binding = (struct internal_binding_t *) data;

   debug_printf(3, "Trying to re-bind %s from tool %s after dlopen\n",
                binding->user_binding->name, binding->associated_binding_table->tool->tool_name);
   
   while (binding->next_binding) {
      binding = binding->next_binding;
      debug_printf(3, "Selecting new innermost version of binding %s from tool %s.\n",
                   binding->user_binding->name, binding->associated_binding_table->tool->tool_name);
   }
   
   result = prepare_symbol(binding);
   if (result == -1) {
      debug_printf(3, "Still could not prepare binding %s after dlopen\n", binding->user_binding->name);
      return 0;
   }

   removefrom_hashtable(&notfound_binding_table, key);
   return 0;
}

static void* dlopen_wrapper(const char* filename, int flags) {
   void *handle;
   debug_printf(1, "User called dlopen(%s, 0x%x)\n", filename, (unsigned int) flags);
   handle = orig_dlopen(filename,flags);

   debug_printf(2, "Searching new dlopened libraries for previously-not-found exports\n");
   foreach_hash_entry(&notfound_binding_table, NULL, per_binding);

   debug_printf(2, "Updating GOT entries for new dlopened libraries\n");
   update_all_library_gots(&function_hash_table);
  
   return handle;
}

static void* dlsym_wrapper(void* handle, const char* symbol_name){
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
      for(loop=0;loop<tool_iter->internal_bindings_size;loop++){
        if(gotcha_strcmp(tool_iter->internal_bindings->user_binding[loop].name,symbol_name)==0){
          free_reverse_iterator(rev);
          return tool_iter->internal_bindings->user_binding[loop].wrapper_pointer;
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

