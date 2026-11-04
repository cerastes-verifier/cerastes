// expect: rejected cerastes-annotation_reserved_library @ L7
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6: the same in C++ with C language linkage written
// extern "C" makes the declaration the library's function.
#include <cerastes.h>

extern "C" void exit(int status) CERASTES_REQUIRES(status == 0);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
