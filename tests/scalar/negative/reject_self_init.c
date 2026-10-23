// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 1: a local initialized from itself stays rejected (0012 item 3)
#include <cerastes.h>

int f(int a)
{
  int x = x;
  return a;
}
