// expect: error division-by-zero @ L21
// mode: c23, c++20
// twin: tests/scalar/call_comma_floor.c
// Pins: 0021: without the comma the two arguments are unsequenced, and C may divide before the call that never returns
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int take2(int v, int w)
    CERASTES_ENSURES(r, r == v)
{
  return v + (w - w);
}

int floor_test(int k, int d)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, false)
{
  return take2(never_returns(k), 100 / d);
}
