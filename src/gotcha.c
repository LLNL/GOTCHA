#include <gotcha/gotcha.h>
#include <gotcha/gotcha_utils.h>
enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* names, int num_actions){
  gotcha_prepare_symbols(names, num_actions);
  int i =0;
  struct link_map * lib_iter = _r_debug.r_map;
  for(;lib_iter != 0 ; lib_iter = lib_iter->l_next){
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, names,num_actions);
  }
  //for(i=0;i<num_actions;i++){
  //  *(void**)(names[i].function_address_pointer) = *(void**)names[i].function_address_pointer;
  //}
  return GOTCHA_SUCCESS;
}
