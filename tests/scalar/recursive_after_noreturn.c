// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 termination (B): never_returns(0) + h(n - 1): the recursive call decreases, whichever order C picks
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int h(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + h(n - 1);
}
