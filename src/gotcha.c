#include <gotcha/gotcha.h>
#include <gotcha/gotcha_utils.h>
enum gotcha_error_t gotcha_wrap(struct gotcha_binding_t* bindings, int num_actions, char* tool_name){
  gotcha_prepare_symbols(bindings, num_actions);
  int i = 0;
  struct link_map* lib_iter = _r_debug.r_map;
  for (; lib_iter != 0; lib_iter = lib_iter->l_next) {
    FOR_EACH_PLTREL(lib_iter, gotcha_wrap_impl, lib_iter, bindings, num_actions);
  }
  enum gotcha_error_t ret_code = GOTCHA_SUCCESS;
  for(i = 0; i<num_actions;i++){
    if(bindings[i].function_address_pointer==0){
      ret_code = GOTCHA_FUNCTION_NOT_FOUND;
    }
  }
  return ret_code;
}
