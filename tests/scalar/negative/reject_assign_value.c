// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 4: an assignment used as a value is rejected
#include <cerastes.h>

int chain(int a)
{
  int y = 0;
  int z = (y = a) + 1;
  return z + y;
}
