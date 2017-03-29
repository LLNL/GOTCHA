#include <gotcha/sampleLib.h>
struct gotcha_binding_t* bindings;
int (*origRetX)(int) = NULL;
int sample_init(){
   char* sample_names[1] = {"retX"};
   bindings = gotcha_prepare_symbols(sample_names,1);
   void* wrap_these[1] = {&dogRetX};
   void** original_calls[1] = {&origRetX};
   gotcha_wrap(bindings, wrap_these, original_calls, 1);
   return 0;
}
int dummyRetX(int foo){
 //never called
 return foo;
}
int retX(int x){return x;}

int dogRetX(int x){
  printf("SO I FOR ONE THINK DOGS SHOULD RETURN %i\n",x);
  return origRetX ? origRetX(x) : 0;
}
void* dog_malloc(int size){
  //printf("SO I FOR ONE THINK WE NEED MORE MENTIONS OF DOGS IN OUR MALLOCS AND PEOPLE WHO DON'T ARE WRONG\n");
  return malloc(size);
}
void* mylloc(int size){
  return malloc(size);
}
