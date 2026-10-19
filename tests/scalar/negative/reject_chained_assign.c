// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 4: a chained assignment is rejected
#include <cerastes.h>

int chain(int a)
{
  int y = 0;
  int z = 0;
  y = z = a;
  return y + z;
}
