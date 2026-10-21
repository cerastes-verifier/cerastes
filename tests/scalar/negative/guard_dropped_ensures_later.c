// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: `r != 0 &&` dropped from an earlier ensures; the division in the later one lost its guard
#include <cerastes.h>

int by_two_ensures(int x)
    CERASTES_REQUIRES(x > 0 && x <= 100)
    CERASTES_ENSURES(r, r <= 100)
    CERASTES_ENSURES(r, 100 / r >= 1)
{
  return x;
}
