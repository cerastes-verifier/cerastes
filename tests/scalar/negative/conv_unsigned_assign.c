// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: u = x with x == -1 is a conversion error at the assignment
#include <cerastes.h>

unsigned from_assign(int x)
    CERASTES_REQUIRES(x == -1)
{
  unsigned u = 0u;
  u = x;
  return u;
}
