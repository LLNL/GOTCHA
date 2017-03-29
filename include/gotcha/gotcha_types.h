#ifndef GOTCHA_TYPES_H
#define GOTCHA_TYPES_H
struct gotcha_binding_t {
  char* name;
  void* function_address_pointer;
  void* wrapper_pointer;
};
#endif
