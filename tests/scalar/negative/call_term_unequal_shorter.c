// expect: error termination @ L21
// mode: c23, c++20
// twin: tests/scalar/call_term_unequal.c
// Pins: 0021 item 4: the callee's measure (n) is shorter than the caller's (n, k) and agrees on its whole length, so it does not decrease; Silicon alone would never check the call behind never_returns
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false);

int f(int n, int k)
    CERASTES_REQUIRES(0 <= n && 0 <= k)
    CERASTES_DECREASES(n, k)
    CERASTES_ENSURES(r, false)
{
  return never_returns(0) + g(n);
}

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return f(n - 1, 0);
}
