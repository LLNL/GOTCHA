#include <elf.h>
#include <link.h>
#include <errno.h>
#include <assert.h>

#include "sampleLib.h"

#include <sys/mman.h>
#include <stdio.h>
void dbg(){}
int main(){
  sample_init();
  int check_val = retX(9);
  assert(check_val == 9);
  return 0;
}

