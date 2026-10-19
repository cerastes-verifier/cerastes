// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/narrow_cast.c
// Pins: 0010 item 5: (signed char)128 does not fit, even written as a cast
#include <cerastes.h>

signed char narrow(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 128)
    CERASTES_ENSURES(r, r == x)
{
  return (signed char)x;
}
