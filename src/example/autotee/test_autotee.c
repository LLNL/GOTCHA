#include <stdio.h>
#include <string.h>

extern int init_autotee(char *filename);
extern int close_autotee();

#define OUTPUT_FILE "tee.out"

int main(int argc, char *argv[])
{
   int result;

   printf("Every stdout print after this line should also appear in %s:\n", OUTPUT_FILE);

   result = init_autotee(OUTPUT_FILE);
   if (result != 0)
      return -1;

   printf("First line\n");
   printf("Second %s\n", "line");
   fprintf(stdout, "Third line\n");
   fprintf(stdout, "%s line\n", "Forth");
   puts("Fifth line");
   fputs("Sixth ", stdout);
   fputs("line\n", stdout);
   fwrite("Seventh line\n", 1, strlen("Seventh line\n"), stdout);
   fprintf(stderr, "Eighth line is stderr and should not appear in in %s\n", OUTPUT_FILE);
   close_autotee();
   printf("Ninth line is after close and should not appear in %s\n", OUTPUT_FILE);

   return 0;
}
