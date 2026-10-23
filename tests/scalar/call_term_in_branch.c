// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: the explicit check sits inside the branch of &&: the recursive call decreases, whichever of never_returns and g C evaluates first
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0)
    CERASTES_ENSURES(r, false);

int g(int a, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return never_returns(0);
  }
  return a && (never_returns(0) + g(a, n - 1) > 0);
}

int run(int a, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == 0)
{
  return g(a, n);
}
