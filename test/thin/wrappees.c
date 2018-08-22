#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wrappees.h"

#define STR2(X) #X
#define STR(X) STR2(X)

int many_params(int one, double two, float three, char four, char *five, long six, void *seven, short eight, struct nine_t nine, int *ten, long eleven_twelve[2])
{
#define CHECK(comparison, val, print_str) do {                          \
      if (comparison) {                                                 \
         fprintf(stderr, "many_params error: " STR(val)                 \
                 " has unexpected value " print_str "\n", val);         \
         return -1;                                                     \
      }                                                                 \
   } while (0)
   
   CHECK(one != 1, one, "%d");
   CHECK(two <= 1.9l || two >= 2.1l, two, "%lf");
   CHECK(three <= 2.9 || three >= 3.1, three, "%f");
   CHECK(four != 4, four, "%uc");
   CHECK(strcmp(five, "five") != 0, five, "%s");
   CHECK(six != 6, six, "%ld");
   CHECK(seven != (void*) 0x7, seven, "%p");
   CHECK(eight != 8, eight, "%hd");
   CHECK(nine.nine != 9, nine.nine, "%lu");
   CHECK(*ten != 10, *ten, "%d");
   CHECK(eleven_twelve[0] != 11, eleven_twelve[0], "%ld");
   CHECK(eleven_twelve[1] != 12, eleven_twelve[1], "%ld");
   return 0;
}

int bigparam(struct bigparam_t bp)
{
   int i;
   for (i = 0 ; i < 1024; i++) {
      if (bp.buffer[i] != i) {
         fprintf(stderr, "Error - bigparam buffer[%d] was %d, expected %d\n",
                 i, (int) bp.buffer[i], i);
         return -1;
      }
   }
   return 0;
}

volatile int zero = 0;
extern int call_recurse(int num);

int recurse(int num, call_recurse_t cr)
{
   if (num == 0)
      return 0;
   int ret = call_recurse(num - 1);
   if (zero != 0) {
      fprintf(stderr, "Error, zero was not zero.  What?");
      exit(-1);
   }
   return ret;
}

double fp_func(double one, double two, double three, double four,
               double five, double six, double seven, double eight,
               double nine, double ten)
{
   return one + two + three + four + five + six + seven + eight + nine + ten;
}
