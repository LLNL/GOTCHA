#include <stdio.h>
#include <stdlib.h>
#include "wrappees.h"
#include "gotcha/gotcha.h"

int call_recurse(int num)
{
   int ret = recurse(num, call_recurse);
   if (ret != 0) {
      fprintf(stderr, "Error, return value of recurse was %d, not zero", ret);
      exit(-1);
   }
   return 0;
}

void call_big_param()
{
   struct bigparam_t bp;
   int i, ret;
   for (i = 0; i < 1024; i++) {
      bp.buffer[i] = i;
   }
   ret = bigparam(bp);
   if (ret != 0) {
      fprintf(stderr, "Error - bigparam returned %d, expected 0\n", ret);
      exit(-1);
   }
}

void call_many_params()
{
   long eleven_twelve[2];
   struct nine_t nine;
   int ret;
   int ten;

   nine.nine = 9;
   ten = 10;
   eleven_twelve[0] = 11;
   eleven_twelve[1] = 12;

   ret = many_params(1, 2.0f, 3.0, 4, "five", 6, (void*) 7, 8,
                     nine, &ten, eleven_twelve);
   if (ret != 0) {
      fprintf(stderr, "error - many_params test returned %d\n", ret);
      exit(-1);
   }
}

void call_fp_func()
{
   double val = fp_func(1.0d, 2.0d, 3.0d, 4.0d, 5.0d, 6.0d, 7.0d, 8.0d, 9.0d, 10.0d);
   if (val <= 44.9 || val >= 55.1) {
      fprintf(stderr, "Incorrect return value %lf of fp_func, expected 55.0\n", val);
      exit(-1);
   }
}

static int pre_called = 0;
static int post_called = 0;

void pretramp(gotcha_wrappee_handle_t handle, void **opaque)
{
   printf("pre-call for function %s\n", gotcha_get_wrappee_name(handle));
   pre_called++;
   *opaque = (void *) 0x4;
}

void posttramp(gotcha_wrappee_handle_t handle, void *opaque)
{
   printf("post-call for function %s\n", gotcha_get_wrappee_name(handle));
   post_called++;
   if (opaque != (void *) 0x4) {
      fprintf(stderr, "Error, did not properly pass-through opaque value\n");
      exit(-1);
   }
}

static gotcha_wrappee_handle_t many_params_handle;
static gotcha_wrappee_handle_t bigparam_handle;
static gotcha_wrappee_handle_t recurse_handle;
static gotcha_wrappee_handle_t fp_handle;

struct gotcha_sigfree_binding_t bindings[] = {
   { "many_params", pretramp, posttramp, &many_params_handle },
   { "bigparam", pretramp, posttramp, &bigparam_handle },
   { "recurse", pretramp, posttramp, &recurse_handle },
   { "fp_func", pretramp, posttramp, &fp_handle },
   { NULL, NULL, NULL, NULL }
};


int main(int argc, char *argv[])
{
   call_many_params();
   call_big_param();
   call_recurse(7);
   call_fp_func();   
   
   if (pre_called != 0) {
      fprintf(stderr, "Expected pre_called (%d) to equal 0\n", pre_called);
      return -1;
   }
   if (post_called != 0) {
      fprintf(stderr, "Expected post_called (%d) to equal 0\n", post_called);
      return -1;
   }
   
   gotcha_sigfree_wrap(bindings, 4, "sigfree_test");

   call_many_params();
   call_big_param();
   call_recurse(7);
   call_fp_func();

   int correct_val = 11;
   if (pre_called != correct_val) {
      fprintf(stderr, "Expected pre_called (%d) to equal %d\n", pre_called, correct_val);
      return -1;
   }
   if (post_called != correct_val) {
      fprintf(stderr, "Expected post_called (%d) to equal %d\n", post_called, correct_val);
      return -1;
   }
   
   return 0;
}
