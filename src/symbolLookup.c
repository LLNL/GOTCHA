#include <elf.h>
#include <link.h>
#include <errno.h>
#include <assert.h>

#include "sampleLib.h"
#include "gnuy_lookups.h"
#include "gotcha_utils.h"

#include <sys/mman.h>
#include <stdio.h>
void dbg(){}
int main(){
  sample_init();
  retX(9);
  return 0;
  //malloc_addresses = gotcha_list_alloc(100);
  //dog_malloc(0);
}

