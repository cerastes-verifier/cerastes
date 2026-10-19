// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 4: the comma operator is rejected
#include <cerastes.h>

int last(int a, int b)
{
  return ((void)a, b);
}
