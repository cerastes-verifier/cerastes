// expect: error conversion @ L16
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: passing x == -1 to an unsigned parameter is a conversion error at the call
#include <cerastes.h>

unsigned take(unsigned a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

unsigned from_arg(int x)
    CERASTES_REQUIRES(x == -1)
{
  return take(x);
}
