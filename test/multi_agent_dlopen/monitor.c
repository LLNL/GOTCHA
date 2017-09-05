/*
 *  monitor.c  -->  libmon.so
 *
 *  Override dlopen() use dlsym(RTLD_NEXT).
 */

#define _GNU_SOURCE  1

#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <gotcha/gotcha.h>

#define MYNAME  "libmon.so"

typedef void *dlopen_fcn_t(const char *, int);

void*(*reel_dlopen)(const char*, int);

void *
wrap_dlopen(const char *file, int flag)
{
    fprintf(stderr, "%s:  enter dlopen:  file = %s\n", MYNAME, file);

    void *ans = reel_dlopen ? (reel_dlopen)(file, flag) : NULL;

    fprintf(stderr, "%s:  exit  dlopen:  handle = %p\n", MYNAME, ans);

    return ans;
}

struct gotcha_binding_t binds[] = {
  { "dlopen", wrap_dlopen, &reel_dlopen}
};
__attribute__((constructor)) void fix_things(){
  gotcha_wrap(binds, 1, "silly");
}
