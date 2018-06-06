/*
 *  Try dlopen(libm.so, ...) and call sin().
 */

#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

#define MYNAME  "main"

typedef double sin_fcn_t(double);

int
main(int argc, char **argv)
{
    sin_fcn_t *sin_fcn = NULL;
    double val = 4.0;
    double ans = 0.0;

    fprintf(stderr, "%s:  val = %.6f\n", MYNAME, val);

    void *handle = dlopen("libm.so", RTLD_NOW);
    if (handle == NULL) {
	err(1, "dlopen failed");
    }

    fprintf(stderr, "%s:\n", MYNAME);

    sin_fcn = dlsym(handle, "sin");
    if (sin_fcn == NULL) {
	err(1, "dlsym failed");
    }

    fprintf(stderr, "%s:\n", MYNAME);

    ans = (*sin_fcn)(val);

    fprintf(stderr, "%s:  ans = %.6f\n", MYNAME, ans);

    return 0;
}
