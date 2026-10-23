// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 and 0012 item 6: with a callee whose result is checked (y != 0), a call that never returns hides nothing: the division is checked before the call that C may evaluate second
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int nonzero(int y)
    CERASTES_REQUIRES(y != 0)
    CERASTES_ENSURES(r, r == y && r != 0)
{
  return y;
}

int divide(int k, int y)
    CERASTES_REQUIRES(0 <= k && y != 0)
    CERASTES_ENSURES(r, false)
{
  return never_returns(k) + 100 / nonzero(y);
}
