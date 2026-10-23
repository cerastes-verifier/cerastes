// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: the measure of the callee is taken over the argument's value, which the comma computes (m = n - 1), at the position of the explicit check
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  int m;
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + g((m = n - 1, m));
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return g(n);
}
