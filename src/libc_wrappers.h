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

#if !defined(LIBC_WRAPPERS_H_)
#define LIBC_WRAPPERS_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#if defined(GOTCHA_USE_LIBC) && !defined(BUILDING_LIBC_WRAPPERS)

#define gotcha_malloc(A)          malloc(A)
#define gotcha_realloc(A, B)      realloc(A, B)
#define gotcha_free(A)            free(A)
#define gotcha_memcpy(A, B, C)    memcpy(A, B, C)
#define gotcha_strncmp(A, B, C)   strncmp(A, B, C)
#define gotcha_strstr(A, B)       strstr(A, B)
#define gotcha_assert(A)          assert(A)
#define gotcha_strcmp(A, B)       strcmp(A, B)
#define gotcha_getenv(A)          getenv(A)
#define gotcha_getpid()           getpid()
#define gotcha_getpagesize()      getpagesize()
#define gotcha_open(A, B, ...)    open(A, B, __VA_ARGS__)
#define gotcha_mmap(A, B, C, D, E, F) mmap(A, B, C, D, E, F)
#define gotcha_atoi(A)            atoi(A)
#define gotcha_close(A)           close(A)
#define gotcha_mprotect(A, B, C)  mprotect(A, B, C)
#define gotcha_read(A, B, C)      read(A, B, C)
#define gotcha_dbg_printf(A, ...) fprintf(stderr, A, __VA_ARGS__)
pid_t gotcha_gettid();            //No libc gettid, always use gotcha version

#else

void *gotcha_malloc(size_t size);
void *gotcha_realloc(void* buffer, size_t size);
void gotcha_free(void* free_me);
void gotcha_memcpy(void* dest, void* src, size_t size);
int gotcha_strncmp(const char* in_one, const char* in_two, int max_length);
char *gotcha_strstr(char* searchIn, char* searchFor);
int gotcha_strcmp(const char* in_one, const char* in_two);
char *gotcha_getenv(const char *env);
pid_t gotcha_getpid();
pid_t gotcha_gettid();
int gotcha_getpagesize();
int gotcha_open(const char *pathname, int flags, ...);
void *gotcha_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
int atoi(const char *nptr);
int close(int fd);
int gotcha_mprotect(void *addr, size_t len, int prot);
ssize_t gotcha_read(int fd, void *buf, size_t count);
ssize_t gotcha_write(int fd, const void *buf, size_t count);
int gotcha_int_printf(int fd, const char *format, ...);
#define gotcha_dbg_printf(FORMAT, ...) gotcha_int_printf(2, FORMAT, __VA_ARGS__)

#define gotcha_assert(A)                                          \
   do {                                                           \
      if (! (A) )                                                 \
         gotcha_assert_fail("" #A, __FILE__, __LINE__, __func__); \
   } while (0); 

#endif

#endif
