/*
 *  dlsym.c  -->  libsym.so
 *
 *  Override dlsym() and replace with __libc_dlsym().
 */

#define _GNU_SOURCE  1

#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>

#define MYNAME  "libsym.so"

typedef void * dlsym_fcn_t(void *, const char *);
typedef void * dlopen_mode_fcn_t(const char *, int);

dlsym_fcn_t __libc_dlsym;
dlopen_mode_fcn_t __libc_dlopen_mode;

void *
dlsym(void *handle, const char *symbol)
{
    fprintf(stderr, "%s:  enter dlsym:  sym = %s\n", MYNAME, symbol);

    void * dl_handle = __libc_dlopen_mode("libdl.so", RTLD_LAZY);

    if (dl_handle == NULL) {
	err(1, "__libc_dlopen_mode failed");
    }

    dlsym_fcn_t * the_dlsym = __libc_dlsym(dl_handle, "dlsym");

    if (the_dlsym == NULL) {
	err(1, "__libc_dlsym failed");
    }

    fprintf(stderr, "%s:  mid   dlsym:  dlsym = %p\n", MYNAME, the_dlsym);

#if 0
    if (handle == RTLD_NEXT) { handle = RTLD_DEFAULT; }
#endif

    void *ans = the_dlsym(handle, symbol);

    if (ans == NULL) {
	err(1, "the_dlsym failed");
    }

    fprintf(stderr, "%s:  exit  dlsym:  ans = %p\n", MYNAME, ans);

    return ans;
}
