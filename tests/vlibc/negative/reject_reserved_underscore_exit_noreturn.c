// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6); the `.cpp` twin writes extern "C"
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6: a written `_Noreturn` on a reserved name outside vlibc is rejected ("does not return" comes from vlibc)
// _Exit's noreturn is vlibc's `ensures(false)`.
#include <cerastes.h>

_Noreturn void _Exit(int status);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
