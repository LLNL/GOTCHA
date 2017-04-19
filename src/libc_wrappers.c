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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <syscall.h>

void *gotcha_malloc(size_t size) 
{
   return malloc(size); 
}

void *gotcha_realloc(void* buffer, size_t size) 
{
   return realloc(buffer,size);
}

void gotcha_free(void *free_me)
{
   free(free_me); 
}

void gotcha_memcpy(void *dest, void *src, size_t size)
{
  memcpy(dest, src, size);
}

int gotcha_strncmp(const char *in_one, const char *in_two, int max_length)
{
  int i = 0;
  for (; i < max_length; i++) {
    if (in_one[i] == '\0') {
      return (in_two[i] == '\0') ? 0 : 1;
    }
    if (in_one[i] != in_two[i]) {
      return in_one[i] - in_two[i];
    }
  }
  return 0;
}

char *gotcha_strstr(char *searchIn, char *searchFor)
{
  return strstr(searchIn, searchFor);
}

void gotcha_assert(int assertion)
{ 
   assert(assertion); 
}

int gotcha_strcmp(const char *in_one, const char *in_two)
{
  int i = 0;
  for (;; i++) {
    if (in_one[i] == '\0') {
      return (in_two[i] == '\0') ? 0 : 1;
    }
    if (in_one[i] != in_two[i]) {
      return in_one[i] - in_two[i];
    }
  }
  return 0;
}

char *gotcha_getenv(const char *name) 
{
   return getenv(name);
}

pid_t gotcha_getpid()
{
   return getpid();
}

pid_t gotcha_gettid()
{
   return syscall(SYS_gettid);
}
