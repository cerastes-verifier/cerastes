// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0010 item 9: the bitwise & is rejected (M7)
#include <cerastes.h>

int combine(int a, int b)
{
  return a & b;
}
