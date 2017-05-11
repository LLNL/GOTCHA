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
#define BUILDING_LIBC_WRAPPERS

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <syscall.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/syscall.h>

#include "libc_wrappers.h"
#include "gotcha_auxv.h"

typedef struct malloc_header_t {
   size_t size;
} malloc_header_t;

typedef struct malloc_link_t {
   malloc_header_t header;
   struct malloc_link_t *next;
} malloc_link_t;

#define MIN_SIZE (sizeof(malloc_link_t) - sizeof(malloc_header_t))
#define MIN_BLOCK_SIZE (1024*32)

static malloc_link_t *free_list = NULL;

static void split_allocation(malloc_link_t *allocation, size_t new_size)
{
   size_t orig_size = allocation->header.size;
   malloc_link_t *newalloc;
   
   if (orig_size - new_size <= sizeof(malloc_link_t))
      return;

   allocation->header.size = new_size;
   newalloc = (malloc_link_t *) (((unsigned char *) &allocation->next) + new_size);
   newalloc->header.size = (orig_size - new_size) - sizeof(malloc_header_t);
   newalloc->next = free_list;
   free_list = newalloc;
}

void *gotcha_malloc(size_t size) 
{
   malloc_link_t *cur, *prev, *newalloc;
   malloc_link_t *best_fit = NULL, *best_fit_prev;
   ssize_t best_fit_diff, diff, block_size;
   void *result;
   
   if (size < MIN_SIZE)
      size = MIN_SIZE;
   if (size % 8)
      size += 8 - (size % 8);

   //Find the tightest fit allocation in the free list
   for (prev = NULL, cur = free_list; cur; cur = cur->next) {
      diff = cur->header.size - size;
      if (diff >= 0 && (!best_fit || diff < best_fit_diff)) {
         best_fit = cur;
         best_fit_prev = prev;
         best_fit_diff = diff;
         if (!diff)
            break;
      }
      prev = cur;
   }
   
   //Removes the best fit from the free list, split if needed, and return
   if (best_fit) {
      if (best_fit_prev)
         best_fit_prev->next = best_fit->next;
      else
         free_list = best_fit->next;
      split_allocation(best_fit, size);
      return (void *) &best_fit->next;
   }

   //Create a new allocation area
   if (size + sizeof(malloc_header_t) > MIN_BLOCK_SIZE) {
      block_size = size + sizeof(malloc_header_t);
      diff = block_size % gotcha_getpagesize();
      if (diff)
         block_size += gotcha_getpagesize() - diff;
   }
   else {
      block_size = MIN_BLOCK_SIZE;
   }

   result = gotcha_mmap(NULL, block_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
   if (result == MAP_FAILED)
       return NULL;
   newalloc = (malloc_link_t *) result;
   newalloc->header.size = block_size - sizeof(malloc_header_t);
   split_allocation(newalloc, size);
   return (void *) &newalloc->next;
}

void *gotcha_realloc(void* buffer, size_t size) 
{
   void *newbuffer;
   malloc_link_t *alloc;

   alloc = (malloc_link_t *) (((malloc_header_t *) buffer) - 1);

   if (size <= alloc->header.size)
      return buffer;

   newbuffer = gotcha_malloc(size);
   if (!newbuffer)
      return NULL;
   gotcha_memcpy(newbuffer, buffer, alloc->header.size);

   gotcha_free(buffer);

   return newbuffer;
}

void gotcha_free(void *buffer)
{
   malloc_link_t *alloc;
   alloc = (malloc_link_t *) (((malloc_header_t *) buffer) - 1);

   alloc->next = free_list;
   free_list = alloc;
}

void gotcha_memcpy(void *dest, void *src, size_t size)
{
   size_t i;
   for (i = 0; i < size; i++) {
      ((unsigned char *) dest)[i] = ((unsigned char *) src)[i];
   }
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
}

char *gotcha_strstr(char *searchIn, char *searchFor)
{
   int i, j;
   if (!searchFor[0])
      return NULL;

   for (i = 0; searchIn[i]; i++) {
      if (searchIn[i] != searchFor[0])
         continue;
      for (j = 1; ; j++) {
         if (!searchFor[j])
            return searchFor + i;
         if (!searchIn[i+j])
            return NULL;
         if (searchFor[j] != searchIn[i+j])
            break;
      }
   }
   return NULL;
}

ssize_t gotcha_write(int fd, const void *buf, size_t count)
{
   return syscall(SYS_write, fd, buf, count);
}

size_t gotcha_strlen(const char *s)
{
   size_t i;
   for (i = 0; s[i]; i++);
   return i;
}

static int ulong_to_hexstr(unsigned long num, char *str, int strlen)
{
   int len, i;
   unsigned long val;
   
   if (num == 0) {
      if (strlen < 2)
         return -1;
      str[0] = '0';
      str[1] = '\0';
      return 1;
   }

   for (len = 0, val = num; val; val = val / 16, len++);
   if (len + 1 >= strlen)
      return -1;

   str[len] = '\0';
   val = num;
   for (i = 1; i <= len; i++) {

      str[len - i] = (val % 16 <= 9) ? ('0' + val % 16) : ('a' + (val % 16 - 10));
      val = val / 16;
   }
   return len;
}

static int ulong_to_str(unsigned long num, char *str, int strlen)
{
   int len, i;
   unsigned long val;
   
   if (num == 0) {
      if (strlen < 2)
         return -1;
      str[0] = '0';
      str[1] = '\0';
      return 1;
   }

   for (len = 0, val = num; val; val = val / 10, len++);
   if (len + 1 >= strlen)
      return -1;

   str[len] = '\0';
   val = num;
   for (i = 1; i <= len; i++) {
      str[len - i] = '0' + val % 10;
      val = val / 10;
   }
   return len;
}

static int slong_to_str(signed long num, char *str, int strlen)
{
   int result;
   if (num >= 0)
      return ulong_to_str((unsigned long) num, str, strlen);

   result = ulong_to_str((unsigned long) (num * -1), str+1, strlen-1);
   if (result == -1)
      return -1;
   str[0] = '-';
   return result + 1;   
}

void gotcha_assert_fail(const char *s, const char *file, unsigned int line, const char *function)
{
   char linestr[64];
   int result;

   result = ulong_to_str(line, linestr, sizeof(linestr)-1);
   if (result == -1)
      linestr[0] = '\0';

   gotcha_write(2, file, gotcha_strlen(file));
   gotcha_write(2, ":", 1);
   gotcha_write(2, linestr, gotcha_strlen(linestr));
   gotcha_write(2, ": ", 2);
   gotcha_write(2, function, gotcha_strlen(function));
   gotcha_write(2, ": Assertion `", 13);
   gotcha_write(2, s, gotcha_strlen(s));
   gotcha_write(2, "' failed.\n", 10);
   syscall(SYS_kill, gotcha_getpid(), SIGABRT);
}

extern char **__environ;
char *gotcha_getenv(const char *name) 
{
   char **s;
   int name_len;

   name_len = gotcha_strlen(name);
   for (s = __environ; *s; s++) {
      if (gotcha_strncmp(name, *s, name_len) != 0)
         continue;
      if ((*s)[name_len] != '=')
         continue;
      return (*s) + name_len + 1;
   }
   return NULL;
}

pid_t gotcha_getpid()
{
   return syscall(SYS_getpid);
}

pid_t gotcha_gettid()
{
   return syscall(SYS_gettid);
}

int gotcha_getpagesize()
{
   static int pagesz = 0;
   if (pagesz)
      return pagesz;

   pagesz = get_auxv_pagesize();
   if (!pagesz)
      pagesz = 4096;
   return pagesz;
}

int gotcha_open(const char *pathname, int flags, ...)
{
   mode_t mode;
   va_list args;
   long result;

   va_start(args, flags);
   if (flags & O_CREAT) {
      mode = va_arg(args, mode_t);
   }
   else {
      mode = 0;
   }
   va_end(args);

   result = syscall(SYS_open, pathname, flags, mode);
   if (result >= 0)
      return (int) result;
   
   return -1;
}

void *gotcha_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
   long result;

   result = syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
   return (void *) result;
}

int gotcha_atoi(const char *nptr)
{
   int neg = 1, len, val = 0, mult = 1;
   const char *cur;

   while (*nptr == '-') {
      neg = neg * -1;
      nptr++;
   }

   for (len = 0; nptr[len] >= '0' && nptr[len] <= '9'; len++);
   
   for (cur = nptr + len - 1; cur != nptr-1; cur--) {
      val += mult * (*cur - '0');
      mult *= 10;
   }

   return val * neg;
}

int gotcha_close(int fd) 
{
   return syscall(SYS_close, fd);
}

int gotcha_mprotect(void *addr, size_t len, int prot)
{
   return syscall(SYS_mprotect, addr, len, prot);
}

ssize_t gotcha_read(int fd, void *buf, size_t count)
{
   return syscall(SYS_read, fd, buf, count);
}

static const char *add_to_buffer(const char *str, int fd, int *pos, char *buffer, int buffer_size, int *num_printed)
{
   int is_escaped = 0;
   for (; *str && *str != '%'; str++) {
      if (!is_escaped && *str == '\\') {
         is_escaped = 1;
         continue;
      }
      if (*pos >= buffer_size) {
         gotcha_write(fd, buffer, buffer_size);
         *num_printed += buffer_size;
         *pos = 0;
      }
      if (is_escaped) {
         switch (*str) {
            case 'a': buffer[*pos] = '\a'; break;
            case 'b': buffer[*pos] = '\b'; break;
            case 'f': buffer[*pos] = '\f'; break;
            case 'n': buffer[*pos] = '\n'; break;
            case 'r': buffer[*pos] = '\r'; break;
            case 't': buffer[*pos] = '\t'; break;
            case 'v': buffer[*pos] = '\v'; break;
            case '\\': buffer[*pos] = '\\'; break;
            case '\'': buffer[*pos] = '\''; break;
            case '\"': buffer[*pos] = '\"'; break;
            case '?': buffer[*pos] = '\?'; break;
            case '0': buffer[*pos] = '\0'; break;
            default: buffer[*pos] = *str;
         }
         is_escaped = 0;
      }
      else {
         buffer[*pos] = *str;
      }
      *pos = *pos + 1;
   }
   return str;
}

int gotcha_int_printf(int fd, const char *format, ...)
{
#define inc(S) do { S++; if (*(S) == '\0') goto done; } while(0)
   va_list args;
   const char *str = format;
   int buffer_pos = 0;
   int char_width, short_width, long_width, long_long_width, size_width;
   int num_printed = 0;
   char buffer[4096];

   va_start(args, format);
   while (*str) {
      str = add_to_buffer(str, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      if (!*str) break;

      gotcha_assert(*str == '%');
      inc(str);

      char_width = short_width = long_width = long_long_width = 0;
      if (*str == 'h' && *(str+1) == 'h') {
         char_width = 1;
         inc(str);
         inc(str);
      }
      else if (*str == 'h') {
         short_width = 1;
         inc(str);
      }
      else if (*str == 'l' && *(str+1) == 'l') {
         long_long_width = 1;
         inc(str);
         inc(str);
      }
      else if (*str == 'l') {
         long_width = 1;
         inc(str);
      }
      else if (*str == 'z') {
         size_width = 1;
         inc(str);
      }

      if (*str == 'd' || *str == 'i') {
         signed long val;
         char numstr[64];
         if (char_width)
            val = (signed long) (signed char) va_arg(args, signed int);
         else if (short_width)
            val = (signed long) (signed short) va_arg(args, signed int);
         else if (long_width)
            val = (signed long) va_arg(args, signed long);
         else if (long_long_width)
            val = (signed long) va_arg(args, signed long long);
         else if (size_width)
            val = (signed long) va_arg(args, ssize_t);
         else
            val = (signed long) va_arg(args, signed int);
         slong_to_str(val, numstr, 64);
         add_to_buffer(numstr, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      else if (*str == 'u' || *str == 'c') {
         unsigned long val;
         char numstr[64];
         if (char_width || *str == 'c')
            val = (unsigned long) (unsigned char) va_arg(args, unsigned int);
         else if (short_width)
            val = (unsigned long) (unsigned short) va_arg(args, unsigned int);
         else if (long_width)
            val = (unsigned long) va_arg(args, unsigned long);
         else if (long_long_width)
            val = (unsigned long) va_arg(args, unsigned long long);
         else if (size_width)
            val = (unsigned long) va_arg(args, ssize_t);
         else
            val = (unsigned long) va_arg(args, unsigned int);
         ulong_to_str(val, numstr, 64);
         add_to_buffer(numstr, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      else if (*str == 'x' || *str == 'X' || *str == 'p') {
         unsigned long val;
         char numstr[64];
         if (*str != 'p') {
            if (char_width)
               val = (unsigned long) (unsigned char) va_arg(args, unsigned int);
            else if (short_width)
               val = (unsigned long) (unsigned short) va_arg(args, unsigned int);
            else if (long_width)
               val = (unsigned long) va_arg(args, unsigned long);
            else if (long_long_width)
               val = (unsigned long) va_arg(args, unsigned long long);
            else if (size_width)
               val = (unsigned long) va_arg(args, ssize_t);
            else
               val = (unsigned long) va_arg(args, unsigned int);
         }
         else {
            val = (unsigned long) va_arg(args, void *);
            add_to_buffer("0x", fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
         }
         ulong_to_hexstr(val, numstr, 64);
         add_to_buffer(numstr, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      else if (*str == 's') {
         char *s = (char *) va_arg(args, char *);
         add_to_buffer(s, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      else if (*str == '%') {
         add_to_buffer("%", fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      else {
         char s[2];
         s[0] = *str;
         s[1] = '\0';
         add_to_buffer(s, fd, &buffer_pos, buffer, sizeof(buffer), &num_printed);
      }
      inc(str);
   }

  done:
   gotcha_write(fd, buffer, buffer_pos);
   num_printed += buffer_pos;
   va_end(args);
   return num_printed;
}

void *gotcha_memset(void *s, int c, size_t n)
{
   int i;
   unsigned char byte = (unsigned char) c;
   for (i = 0; i < n; i++) {
      ((unsigned char *) s)[i] = byte;
   }
   return s;
}
