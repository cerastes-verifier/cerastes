// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: `r < 2147483647 &&` dropped from an ensures; `r + 1` can overflow
#include <cerastes.h>

int by_overflow(int x)
    CERASTES_REQUIRES(x >= 0 && x < 2147483647)
    CERASTES_ENSURES(r, r + 1 > x)
{
  return x;
}
