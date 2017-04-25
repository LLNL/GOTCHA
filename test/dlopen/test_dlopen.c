#include <dlfcn.h>
#include <stdio.h>
#include "gotcha/gotcha.h"

int correct_return_four()
{
   return 4;
}

int return_five() 
{
   /* Intentional bug, gotcha will correct this to return 5*/
   return 3;
}

int correct_return_five()
{
   return 5;
}

static int (*buggy_return_four)(void);
static int (*buggy_return_five)(void);

struct gotcha_binding_t funcs[] = {
   { "return_four", correct_return_four, &buggy_return_four },
   { "return_five", correct_return_five, &buggy_return_five }
};

int main(int argc, char *argv[])
{
   void *libnum;
   int (*retfour)(void);
   int (*test_retfive)(void);
   int had_error = 0;
   int result;

   result = gotcha_wrap(funcs, 2, "dlopen_test");
   if (result != 0) {
      fprintf(stderr, "ERROR: gotcha_wrap returned error code %d\n", result);
      return -1;
   }

   libnum = dlopen("libnum.so", RTLD_NOW);
   if (!libnum) {
      fprintf(stderr, "ERROR: Test failed to dlopen libnum.so\n");
      return -1;
   }

   /* Test 1: Check if a dlsym generated indirect call gets re-routed by gotcha */
   retfour = (int (*)(void)) dlsym(libnum, "return_four");
   if (retfour() != 4) {
      fprintf(stderr, "ERROR: dlsym returned original function, not wrapped\n");
      had_error = -1;
   }

   /* Test 2: Does a call in a dlopen'd library get rerouted by gotcha */
   test_retfive = (int (*)(void)) dlsym(libnum, "test_return_five");
   if (test_retfive() != 5) {
      fprintf(stderr, "ERROR: call to return_five in libnum.so was not wrapped by correct_return_five\n");
      had_error = -1;
   }

   return had_error;
}
