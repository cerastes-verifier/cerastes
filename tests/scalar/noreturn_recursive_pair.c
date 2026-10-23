// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 termination (B): g(n - 1) + g(n - 2) in a function that never returns: both calls decrease
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0 || x == 1)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n < 2) {
    return never_returns(n);
  }
  return g(n - 1) +
         g(n - 2);
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return g(n);
}
