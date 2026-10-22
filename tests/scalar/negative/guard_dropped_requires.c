// expect: error division-by-zero @ L8
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: the guard `x != 0 &&` of a requires dropped (as a drop-conjunct mutant leaves it)
#include <cerastes.h>

int by_requires(int x)
    CERASTES_REQUIRES(100 / x > 1)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}
