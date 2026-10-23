// expect: error termination @ L19
// mode: c23, c++20
// twin: tests/scalar/call_term_in_branch.c
// Pins: 0021 item 4: the call g(a, n) in the branch does not decrease; the check is reported although never_returns hides it from Silicon
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int a, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return never_returns(0);
  }
  return a && (never_returns(0) + g(a, n) > 0);
}
