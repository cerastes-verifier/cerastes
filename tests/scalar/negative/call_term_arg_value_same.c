// expect: error termination @ L20
// mode: c23, c++20
// twin: tests/scalar/call_term_arg_value.c
// Pins: 0021 item 4: with m = n the call does not decrease; Silicon alone never checks it behind never_returns (m = n + 1 cannot be written: g's own precondition would have to hold for n + 1 at every n)
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
  return never_returns(0) + g((m = n, m));
}

int run(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, false)
{
  return g(n);
}
