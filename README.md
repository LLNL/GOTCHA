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
  * Full user docs. Right now you have this README, my email (poliakoff1@llnl.gov) and Doxygen as options for understanding the software. This is meant to be productized software, in the future we hope to give you better documentation resources.
  * Automated wrapper generation. We have a pre-pre-alpha Clang compiler plugin for generating libraries which use GOTCHA
    to wrap all the functions in a header. We hope in the future to automate some of the scut work in GOTCHA use

Quick Start
-----------

*Building* is fairly trivial. In the root directory

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX = <where you want the sofware> ..
make install

*Usage* is fairly simple. For us to wrap a function, we need to know its name, what you want it wrapped with (the wrapper), and we need to give you some ability to call the function you wrapped (wrappee). GOTCHA works on triplets containing this information. For full docs, see [examples](src/example/autotee/autotee.c)


