// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 4: x++ inside an expression is rejected
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x < 1000)
{
  int y = x++;
  return y;
}
