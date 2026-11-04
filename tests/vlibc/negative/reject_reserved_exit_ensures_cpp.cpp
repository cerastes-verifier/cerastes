// expect: rejected cerastes-annotation_reserved_library @ L7
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6 (question 115): the same with C language linkage written, in C++
// extern "C" makes the declaration the library's function, so its contract is rejected.
#include <cerastes.h>

extern "C" void exit(int status) CERASTES_ENSURES(false);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
