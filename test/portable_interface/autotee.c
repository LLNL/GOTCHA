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

/**
 * autotee -
 * Using gotcha, wrap the major IO writing routines with functions that 
 * "tee" any stdout output to another file.  Init by calling
 *    init_autotee(filename)
 * finish by calling:
 *    close_autotee()
 *
 * Note, this is a demonstration program for gotcha and does not handle
 * cases like stdout's file descriptor being dup'd or more esoteric 
 * IO routines.
 **/


#include "gotcha/gotcha_types.h"
#include "gotcha/gotcha.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static int tee_fd = -1;
static FILE *tee_FILE = NULL;

typedef void* gotcha_index_t;

void* gotcha_func_pointer_for_index(gotcha_index_t in){
  return in;
}

static gotcha_index_t orig_printf_index;
static gotcha_index_t orig_fprintf_index;
static gotcha_index_t orig_vfprintf_index;
static gotcha_index_t orig_vprintf_index;
static gotcha_index_t orig_write_index;
static gotcha_index_t orig_puts_index;
static gotcha_index_t orig_fputs_index;
static gotcha_index_t orig_fwrite_index;

static int (*orig_printf)(const char *, ...);
static int (*orig_fprintf)(FILE *, const char *, ...);
static int (*orig_vfprintf)(FILE *, const char *, va_list);
static int (*orig_vprintf)(const char *, va_list);
static ssize_t (*orig_write)(int, const void *, size_t);
static int (*orig_puts)(const char *);
static int (*orig_fputs)(const char *, FILE *);
static int (*orig_fwrite)(const void *, size_t, size_t, FILE *);

static int printf_wrapper(const char *format, ...);
static int fprintf_wrapper(FILE *stream, const char *format, ...);
static int vfprintf_wrapper(FILE *stream, const char *str, va_list args);
static int vprintf_wrapper(const char *str, va_list args);
static ssize_t write_wrapper(int fd, const void *buffer, size_t size);
static int puts_wrapper(const char *str);
static int fputs_wrapper(const char *str, FILE *f);
static int fwrite_wrapper(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#define NUM_IOFUNCS 8
struct gotcha_binding_t iofuncs[] = {
   { "printf", printf_wrapper, &orig_printf_index },
   { "fprintf", fprintf_wrapper, &orig_fprintf_index },
   { "vfprintf", vfprintf_wrapper, &orig_vfprintf_index },
   { "vprintf", vprintf_wrapper, &orig_vprintf_index },
   { "write", write_wrapper, &orig_write_index },
   { "puts", puts_wrapper, &orig_puts_index },
   { "fputs", fputs_wrapper, &orig_fputs_index },
   { "fwrite", fwrite_wrapper, &orig_fwrite_index }
};

int init_autotee(const char *teefile)
{
   enum gotcha_error_t result;

   tee_FILE = fopen(teefile, "w");
   if (!tee_FILE) {
      perror("Failed to open tee file");
      return -1;
   }
   tee_fd = fileno(tee_FILE);

   result = gotcha_wrap(iofuncs, NUM_IOFUNCS, "autotee");
   if (result != GOTCHA_SUCCESS) {
      fprintf(stderr, "gotcha_wrap returned %d\n", (int) result);
      return -1;
   }

   return 0;
}

int close_autotee()
{
   if (tee_FILE) {
      fclose(tee_FILE);
      tee_fd = -1;
   }
   return 0;
}

static int printf_wrapper(const char *format, ...)
{
   int result;
   va_list args, args2;
   va_start(args, format);

   orig_vprintf = gotcha_func_pointer_for_index(orig_vprintf_index);
   orig_vfprintf = gotcha_func_pointer_for_index(orig_vfprintf_index);
   if (tee_FILE) {
      va_copy(args2, args);   
      orig_vfprintf(tee_FILE, format, args2);
      va_end(args2);
   }

   result = orig_vprintf(format, args);   
   va_end(args);

   return result;
}

static int fprintf_wrapper(FILE *stream, const char *format, ...)
{
   int result;
   va_list args, args2;
   va_start(args, format);
   orig_vprintf = gotcha_func_pointer_for_index(orig_vprintf_index);
   orig_vfprintf = gotcha_func_pointer_for_index(orig_vfprintf_index);
   
   if (stream != stdout) {
      result = orig_vfprintf(stream, format, args);
   }
   else {
      if (tee_FILE) {
         va_copy(args2, args);
         orig_vfprintf(tee_FILE, format, args2);
         va_end(args2);
      }
      result = orig_vfprintf(stdout, format, args);
   }

   va_end(args);
   return result;
}

static int vfprintf_wrapper(FILE *stream, const char *str, va_list args)
{
   orig_vfprintf = gotcha_func_pointer_for_index(orig_vfprintf_index);
   va_list args2;
   if (stream != stdout) {
      return orig_vfprintf(stream, str, args);
   }
   if (tee_FILE) {
      va_copy(args2, args);
      orig_vfprintf(tee_FILE, str, args2);
      va_end(args2);
   }
   return orig_vfprintf(stream, str, args);
}

static int vprintf_wrapper(const char *str, va_list args)
{
   orig_vprintf = gotcha_func_pointer_for_index(orig_vprintf_index);
   orig_vfprintf = gotcha_func_pointer_for_index(orig_vfprintf_index);
   va_list args2;
   if (tee_FILE) {
      va_copy(args2, args);
      orig_vfprintf(tee_FILE, str, args2);
      va_end(args2);
   }
   return orig_vprintf(str, args);
}

static ssize_t write_wrapper(int fd, const void *buffer, size_t size)
{
   orig_write = gotcha_func_pointer_for_index(orig_write_index);
   if (fd != 1)
      return orig_write(fd, buffer, size);
   
   if (tee_fd != -1) 
      orig_write(tee_fd, buffer, size);
   return orig_write(fd, buffer, size);
}

static int puts_wrapper(const char *str)
{
   orig_puts = gotcha_func_pointer_for_index(orig_puts_index);
   orig_fputs = gotcha_func_pointer_for_index(orig_fputs_index);
   if (tee_FILE) {
      orig_fputs(str, tee_FILE);
      orig_fputs("\n", tee_FILE);
   }
   return orig_puts(str);
}

static int fputs_wrapper(const char *str, FILE *f)
{
   orig_fputs = gotcha_func_pointer_for_index(orig_fputs_index);
   if (f != stdout)
      return orig_fputs(str, f);
   if (tee_FILE)
      orig_fputs(str, tee_FILE);
   return orig_fputs(str, f);
}

static int fwrite_wrapper(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
   orig_fwrite = gotcha_func_pointer_for_index(orig_fwrite_index);
   if (stream != stdout) 
      return orig_fwrite(ptr, size, nmemb, stream);
   if (tee_FILE)
      orig_fwrite(ptr, size, nmemb, tee_FILE);
   return orig_fwrite(ptr, size, nmemb, stream);
}
