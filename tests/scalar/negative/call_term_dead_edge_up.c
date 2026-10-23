// expect: error termination @ L24
// mode: c23, c++20
// twin: tests/scalar/call_term_dead_edge.c
// Pins: 0021 item 4: g(n + 1) does not decrease, and the dead call f(n) in g still puts f and g in one component
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x == 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false);

int f(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + g(n + 1);
}

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (0) {
    return f(n);
  }
  return never_returns(0);
}
