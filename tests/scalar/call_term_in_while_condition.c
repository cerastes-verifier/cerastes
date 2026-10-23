// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: the explicit check sits in a loop condition (top-test shape): the recursive call decreases, whichever of never_returns and g C evaluates first
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  int i = 0;
  if (n == 0) {
    return never_returns(0);
  }
  CERASTES_INVARIANT(i == 0)
  CERASTES_DECREASES(n - i)
  while (never_returns(i) + g(n - 1) > 0) {
    i = i + 1;
  }
  return i;
}
