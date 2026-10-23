// expect: error termination @ L20
// mode: c23, c++20
// twin: tests/scalar/noreturn_recursive_pair.c
// Pins: 0021 soundness pin: g(n - 1) + g(n) in a function that never returns (g(n + 1) cannot be written: g's own precondition would have to hold for n + 1 at every n): Silicon checks the first call and the path ends there, so the second call's measure is checked by the explicit obligation, at its own line
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
         g(n);
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return g(n);
}
