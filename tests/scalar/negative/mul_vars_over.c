// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/mul_vars.c
// Pins: 0010 item 3: 50000 * 50000 does not fit int
#include <cerastes.h>

int area(int w, int h)
    CERASTES_REQUIRES(w >= 0 && w <= 50000 && h >= 0 && h <= 50000)
    CERASTES_ENSURES(r, r >= 0)
{
  return w * h;
}
