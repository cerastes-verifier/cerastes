// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/mul_const.c
// Pins: 0010 item 3: 715827883 * 3 does not fit int
#include <cerastes.h>

int triple(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 715827883)
    CERASTES_ENSURES(r, r >= a)
{
  return a * 3;
}
