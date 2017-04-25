extern void mark_had_error();
extern int return_five();

int return_four()
{
   /* Intentional bug, gotcha wrapping will correct this to return 4 */
   return 3;
}

int test_return_five()
{
   return return_five();
}


