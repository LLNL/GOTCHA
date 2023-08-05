/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free
Software Foundation) version 2.1 dated February 1999.  This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the terms and conditions of the GNU Lesser General Public License
for more details.  You should have received a copy of the GNU Lesser General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include "gotcha/gotcha.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifndef LIB_NAME_RAW
#define LIB_NAME_RAW libnum.so
#endif
#ifndef LIB2_NAME_RAW
#define LIB2_NAME_RAW libnum2.so
#endif

#define LIB_NAME QUOTE(LIB_NAME_RAW)
#define LIB2_NAME QUOTE(LIB2_NAME_RAW)
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


static gotcha_wrappee_handle_t buggy_return_four;
static gotcha_wrappee_handle_t buggy_return_five;
struct gotcha_binding_t funcs[] = {
   { "return_four", correct_return_four, &buggy_return_four },
   { "return_five", correct_return_five, &buggy_return_five }
};

int main()
{
   void *libnum;
   int (*retfour)(void);
    int (*test_retfive)(void);
    int (*retdummy)(void);
   int had_error = 0;
   int result;

   result = gotcha_wrap(funcs, 2, "dlopen_test");
   if(result != GOTCHA_FUNCTION_NOT_FOUND){
     fprintf(stderr, "GOTCHA should have failed to find a function, but found it\n");
     return -1;
   }

   libnum = dlopen(LIB_NAME, RTLD_NOW);
   if (!libnum) {
      fprintf(stderr, "ERROR: Test failed to dlopen libnum.so\n");
      return -1;
   }
    libnum = dlopen(LIB2_NAME, RTLD_NOW);
    if (!libnum) {
        fprintf(stderr, "ERROR: Test failed to dlopen libnum2.so\n");
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
    /* Test 3: Does the dlsym implementation find the second occurrence of the symbol */
    test_retfive = (int (*)(void)) dlsym(RTLD_NEXT, "test_return_five");
    if (test_retfive == NULL || test_retfive() != 5) {
        fprintf(stderr, "ERROR: call to return_four should not be found in RTLD_NEXT from libnum2.so and return 4\n");
        had_error = -1;
    }
    /* Test 4: Does the dlsym implementation find the first occurrence of the symbol */
    retfour = (int (*)(void)) dlsym(RTLD_DEFAULT, "return_four");
    if (retfour == NULL || retfour() != 4) {
        fprintf(stderr, "ERROR: call to return_four should be found in RTLD_DEFAULT from libnum.so and return 4\n");
        had_error = -1;
    }
    test_retfive = (int (*)(void)) dlsym(RTLD_DEFAULT, "test_return_five");
    if (test_retfive != NULL) {
        fprintf(stderr, "ERROR: call to return_five in libnum.so was not wrapped by correct_return_five\n");
        had_error = -1;
    }
    retdummy = (int (*)(void)) dlsym(RTLD_DEFAULT, "return_dummy");
    if (retdummy != NULL) {
        fprintf(stderr, "ERROR: call to return_dummy should not be found in RTLD_DEFAULT\n");
        had_error = -1;
    }
    retdummy = (int (*)(void)) dlsym(RTLD_NEXT, "return_dummy");
    if (retdummy != NULL) {
        fprintf(stderr, "ERROR: call to return_dummy should not be found in RTLD_NEXT\n");
        had_error = -1;
    }
   return had_error;
}
