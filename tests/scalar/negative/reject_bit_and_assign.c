// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 9: &= is rejected too
#include <cerastes.h>

int mask(int a)
{
  a &= 7;
  return a;
}
