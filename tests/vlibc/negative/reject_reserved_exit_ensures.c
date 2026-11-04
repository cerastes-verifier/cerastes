// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6); the `.cpp` twin writes extern "C"
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6 (question 115): a contract on a reserved name outside vlibc is rejected, with no <stdlib.h> in the unit
// exit's contract comes from vlibc's <stdlib.h> only.
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(false);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
