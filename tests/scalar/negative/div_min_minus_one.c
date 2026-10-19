// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/div_nonzero.c
// Pins: 0010 item 7: INT_MIN / -1 is an overflow
// b != 0 allows -1, and a may be INT_MIN.
#include <cerastes.h>

int divide(int a, int b)
    CERASTES_REQUIRES(b != 0)
{
  return a / b;
}
