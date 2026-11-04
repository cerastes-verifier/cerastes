// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6); the `.cpp` twin writes extern "C"
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6: a `requires` on a reserved name outside vlibc is rejected too (a precondition would restrict every caller)
// A user precondition on exit would be a contract vlibc does not state.
#include <cerastes.h>

void exit(int status) CERASTES_REQUIRES(status == 0);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
