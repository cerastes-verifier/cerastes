// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: two functions that call each other behind never_returns, each decreasing: the explicit checks hold in both
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false);

int f(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + g(n - 1);
}

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + f(n - 1);
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return f(n);
}
