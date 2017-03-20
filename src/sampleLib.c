#include "sampleLib.h"
#include "call_mech.hpp"
struct gotcha_binding_t* bindings;
int sample_init(){
   char** sample_names = gotcha_malloc(sizeof(char*)*1);
   sample_names[0] = "retX";
   bindings = gotcha_prepare_symbols(sample_names,1);
   void** wrap_these = (void**)malloc(sizeof(void*)*1);
   wrap_these[0] = (void*)&dogRetX;
   gotcha_wrap(bindings, wrap_these, 1);
   return 0;
}
int retX(int x){return x;}
int dogRetX(int x){
  printf("SO I FOR ONE THINK DOGS SHOULD RETURN %i\n",x);
  return ((int(*)(int))(bindings[0].function_address_pointer))(x);
}
void* dog_malloc(int size){
  printf("SO I FOR ONE THINK WE NEED MORE MENTIONS OF DOGS IN OUR MALLOCS AND PEOPLE WHO DON'T ARE WRONG\n");
  return malloc(size);
}
void* mylloc(int size){
  return malloc(size);
}
