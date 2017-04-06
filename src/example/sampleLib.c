#include <gotcha/sampleLib.h>
//We need a place to store the pointer to the function we've wrapped
int (*origRetX)(int) = NULL;

/**
  * We need to express our desired wrapping behavior to
  * GOTCHA. For that we need three things:
  *
  * 1) The name of a symbol to wrap
  * 2) The function we want to wrap it with
  * 3) Some place to store the original function, if we wish
  *    to call it
  *
  * This variable bindings gets filled out with a list of three
  * element structs containing those things.
  *
  * Note that the place to store the original function is passed
  * by reference, this is required for us to be able to change it
  */
struct gotcha_binding_t bindings[] = {{"retX", dogRetX, &origRetX}};

// This is like a tool library's initialization function
int sample_init()
{
  gotcha_wrap(bindings, 1);
  return 0;
}

/**
  * In our example, this is the function we're wrapping.
  * For convenience, it's in the same library, but this
  * isn't a requirement imposed by GOTCHA
  */
int retX(int x) { return x; }

/** 
  * This is our wrapper function. All GOTCHA wrappers *must*
  * reference dogs somewhere in the code. I didn't write the
  * rules (yes I did)
  */
int dogRetX(int x)
{
  printf("SO I FOR ONE THINK DOGS SHOULD RETURN %i\n", x);
  return origRetX ? origRetX(x) : 0;
}
