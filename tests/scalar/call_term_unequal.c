// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: the callee's measure (n, k) is longer than the caller's (n) and agrees on n, so it decreases; the explicit check after never_returns matches Silicon's rule for unequal lengths
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0)
    CERASTES_ENSURES(r, false);

int g(int n, int k)
    CERASTES_REQUIRES(0 <= n && 0 <= k)
    CERASTES_DECREASES(n, k)
    CERASTES_ENSURES(r, false);

int f(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  return never_returns(0) + g(n, 0);
}

int g(int n, int k)
    CERASTES_REQUIRES(0 <= n && 0 <= k)
    CERASTES_DECREASES(n, k)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(k - k);
  }
  return f(n - 1);
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return f(n);
}
