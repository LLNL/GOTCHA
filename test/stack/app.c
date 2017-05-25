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

#include <stdio.h>
#include <string.h>
#include "tool1.h"
#include "tool2.h"
#include "lib1.h"

extern int init_autotee(char *filename);
extern int close_autotee();

#define OUTPUT_FILE "tee.out"

int main()
{
   int result;

   result = init_tool1();
   result = init_tool2();
   if (result != 0)
      return -1;

   result = retX(10); 
   printf("Result %d\n", result);
   return (result==13) ? 0 : 1;
}
