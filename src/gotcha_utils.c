#include "gotcha_utils.h"
void* gotcha_malloc(size_t size){
  return malloc(size);
}
void gotcha_free(void** free_me){
  free(free_me);
}
void gotcha_memcpy(void* dest, void* src, size_t size){
  memcpy(dest, src, size);
}
int gotcha_strncmp(const char* in_one, const char* in_two, int max_length){
  int i=0;
  for(;i<max_length;i++){
     if(in_one[i]=='\0'){
       return (in_two[i]=='\0') ? 0 : 1;
     }
     if(in_one[i] != in_two[i]){
       return in_one[i] - in_two[i];
     } 
  }
  return 0;
}
char* gotcha_strstr(char* searchIn, char* searchFor){
  return strstr(searchIn, searchFor);
}
void gotcha_assert(int assertion){
  assert(assertion);
}

int gotcha_strcmp(const char* in_one, const char* in_two){
  int i=0;
  for(;;i++){
     if(in_one[i]=='\0'){
       return (in_two[i]=='\0') ? 0 : 1;
     }
     if(in_one[i] != in_two[i]){
       return in_one[i] - in_two[i];
     } 
  }
  return 0;
}
