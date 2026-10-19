// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/scalar/div_nonzero.c
// Pins: 0010 item 7: division by zero, at the operator
#include <cerastes.h>

int divide(int a, int b)
    CERASTES_REQUIRES(b >= 0)
{
  return a / b;
}
