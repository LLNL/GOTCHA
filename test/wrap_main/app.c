#include <stdio.h>
#include "lib.h"
#include <link.h>
int main(){
  struct link_map* my_iter= _r_debug.r_map;
  for(;my_iter!=NULL;my_iter=my_iter->l_next){
    printf("Have library %s at %lx\n",my_iter->l_name?:"NULL",my_iter->l_addr);

  }
  printf("In main main\n");
  return 1; // Should return 0
}
