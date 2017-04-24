GOTCHA v0.0.1 (pre-alpha)
============

GOTCHA is a library with an API which enables redirection of function calls to shared libraries in 
a running process, with similarities to things like LD_PRELOAD, but with significantly
less possibility for the many issues that come with these shared library approaches. This enables easy
methods of accomplishing tasks like code instrumentation or wholesale replacement of mechanisms in programs
without disrupting their source code.

It is important to note that GOTCHA is very much a work-in-progress. This is pre-alpha software, and
*SHOULD NOT* be distributed, we plan an eventual release on Github at which time this goes away. This
also means bugs may exist, and some functionality may not yet be implemented, including

  * "Stacking," or having several tools each use GOTCHA to wrap the same function. We eventually want to allow this,
    and to have an API in which the order in which wrappers execute is configurable, but we aren't there
    yet.
  * Libc removal. We know our users want to wrap malloc, and we want to remove libc from our dependencies.
  * Full user docs. Right now you have this README, my email (poliakoff1@llnl.gov) and Doxygen as options for understanding the software. This is meant to be productized software, in the future we hope to give you better documentation resources.
  * Automated wrapper generation. We have a pre-pre-alpha Clang compiler plugin for generating libraries which use GOTCHA
    to wrap all the functions in a header file. We hope in the future to automate some of the scut work in GOTCHA use

Quick Start
-----------

*Building GOTCHA* is trivial. In the root directory of the repo

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX = <where you want the sofware> ..
make install
```
*Usage* is fairly simple. For us to wrap a function, we need to know its name, what you want it wrapped with (the wrapper), and we need to give you some ability to call the function you wrapped (wrappee). GOTCHA works on triplets containing this information. We have [small sample uses](src/example/autotee/autotee.c), but the standard workflow looks like


```
  #include <gotcha/gotcha.h>
  static int (*wrappee_puts)(const char*); //this lets you call the original
  static int puts_wrapper(const char* str); //this is the declaration of your wrapper
  static int (*wrappee_fputs)(const char*, FILE*);
  static int fputs_wrapper(const char* str, FILE* f);
  struct gotcha_binding_t wrap_actions [] = {
    { "puts", puts_wrapper, &wrappee_puts },
    { "fputs", fputs_wrapper, &wrappee_fputs },
  } 
  int init_mytool(){
    gotcha_wrap(wrap_actions, sizeof(wrap_actions)/sizeof(struct gotcha_binding_t), "my_tool_name");
  }
  static int fputs_wrapper(const char* str, FILE* f){
    // insert clever tool logic here
    return wrappee_fputs(str, f); //wrappee_fputs was directed to the original fputs by gotcha_wrap
  }

```

*Building your tool* changes little, you just need to add the prefix you installed GOTCHA to your include directories, the location
the library was installed (default is <that_prefix>/lib) to your library search directories (-L...), and link
libgotcha.so (-lgotcha) with your tool. Very often this becomes "add -lgotcha to your link line," and nicer CMake integration is coming down the pipe.

That should represent all the work your application needs to do to use GOTCHA

Contact/Legal
-----------

The license is [LGPL](LGPL).

We need to reemphasize that this software shouldn't be distributed, it has not been released. Thanks very much for testing it out in this rather unstable state, feel free to contact us with questions.

Primary contact/Lead developer

David Poliakoff (poliakoff1@llnl.gov)

Other developers

Matt Legendre  (legendre1@llnl.gov)
