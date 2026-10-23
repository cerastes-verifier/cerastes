// expect: error termination @ L24
// mode: c23, c++20
// twin: tests/scalar/call_term_mutual.c
// Pins: 0021 item 4: f calls g(n), which calls f(n - 1): the measure does not decrease from f to g, and the check behind never_returns is the only one that sees it
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
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
  return never_returns(0) + g(n);
}

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(1);
  }
  return never_returns(0) + f(n - 1);
}
