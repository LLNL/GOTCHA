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
#include "gotcha/gotcha.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifndef LIB_NAME_RAW
#    define LIB_NAME_RAW libnum.so
#endif

#define LIB_NAME QUOTE(LIB_NAME_RAW)

int
correct_return_four()
{
    return 4;
}

int
return_five()
{
    /* Intentional bug, gotcha will correct this to return 5*/
    return 3;
}

int
correct_return_five()
{
    return 5;
}

static gotcha_wrappee_handle_t buggy_return_four;
static gotcha_wrappee_handle_t buggy_return_five;
struct gotcha_binding_t        funcs[] = {
    { "return_four", correct_return_four, &buggy_return_four },
    { "return_five", correct_return_five, &buggy_return_five }
};

int
main()
{
    void* libnum              = NULL;
    int (*retfour)(void)      = NULL;
    int (*retfive)(void)      = NULL;
    int (*test_retfive)(void) = NULL;
    int had_error             = 0;
    int result                = 0;

    result = gotcha_wrap(funcs, 2, "dlopen_test");
    if(result != GOTCHA_FUNCTION_NOT_FOUND)
    {
        fprintf(stderr, "GOTCHA should have failed to find a function, but found it\n");
        return -1;
    }

    libnum = dlopen(LIB_NAME, RTLD_NOW);
    if(!libnum)
    {
        fprintf(stderr, "ERROR: Test failed to dlopen %s\n", LIB_NAME);
        return -1;
    }

    /* Test 1: Check if a dlsym generated indirect call gets re-routed by gotcha */
    retfour = (int (*)(void)) dlsym(libnum, "return_four");
    if(retfour() != 4)
    {
        fprintf(stderr, "ERROR: dlsym returned original function, not wrapped\n");
        had_error = -1;
    }

    /* Test 2: dlsym + RTLD_DEFAULT returns implementation in this file */
    retfive = (int (*)(void)) dlsym(RTLD_DEFAULT, "return_five");
    if(retfive == NULL)
    {
        fprintf(stderr,
                "ERROR: dlsym(RTLD_DEFAULT, 'return_five') returned null pointer\n");
        had_error = -1;
    }
    else if(retfive() != 3)
    {
        fprintf(stderr,
                "ERROR: dlsym(RTLD_DEFAULT, 'return_five') returned the wrong symbol\n");
        had_error = -1;
    }

    /* Test 3: dlsym + RTLD_NEXT returns implementation in num2.c */
    retfive = (int (*)(void)) dlsym(RTLD_NEXT, "return_five");
    if(retfive == NULL)
    {
        fprintf(stderr, "ERROR: dlsym(RTLD_NEXT, 'return_five') returned null pointer\n");
        had_error = -1;
    }
    else if(retfive() != 6)
    {
        fprintf(stderr,
                "ERROR: dlsym(RTLD_NEXT, 'return_five') returned the wrong symbol\n");
        had_error = -1;
    }

    /* Test 4: Does a call in a dlopen'd library get rerouted by gotcha */
    test_retfive = (int (*)(void)) dlsym(libnum, "test_return_five");
    if(test_retfive() != 5)
    {
        fprintf(stderr,
                "ERROR: call to return_five in %s was not wrapped by "
                "correct_return_five\n",
                LIB_NAME);
        had_error = -1;
    }

    return had_error;
}
