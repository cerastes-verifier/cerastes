// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 and 0020 E6: the comma orders never_returns(k) before 100 / d, so the division (which d == 0 would fail) is never reached
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int take(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int floor_test(int k, int d)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, false)
{
  return take((never_returns(k), 100 / d));
}
