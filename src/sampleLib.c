#include <gotcha/sampleLib.h>
int (*origRetX)(int) = NULL;
struct gotcha_binding_t bindings[] = {
                                       {"retX", dogRetX, &origRetX}
                                     };
int sample_init(){
   gotcha_wrap(bindings, 1);
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
