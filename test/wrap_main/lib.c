#include "lib.h"
#include <stdlib.h>
gotcha_wrappee_handle_t orig_main_handle;
int couldnt_find_a_better_main(int argc, char** argv){
  printf("In wrapper main\n");
  typeof(&couldnt_find_a_better_main) orig_main = gotcha_get_wrappee(orig_main_handle);
  int i;
  for(i=0;i<argc;i++){
    printf("%d : %s\n",i,argv[i]);
  }
  int return_code = orig_main(argc, argv) - 1;
  printf("Returning %d\n",return_code);
  return return_code;
}
struct gotcha_binding_t actions[] = {
  {"main", couldnt_find_a_better_main, &orig_main_handle}
};
__attribute__((constructor)) void phnglui(){
  printf("In attr constructor func\n");
  setenv("GOTCHA_DEBUG","3",1); // TODO: DEBUG DELETE
  gotcha_wrap(actions,1,"test_tool");
}
