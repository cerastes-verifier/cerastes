// expect: error termination @ L22
// mode: c23, c++20
// twin: tests/scalar/call_term_in_while_condition.c
// Pins: 0021 item 4: the call g(n) in the loop condition does not decrease
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  int i = 0;
  if (n == 0) {
    return never_returns(0);
  }
  CERASTES_INVARIANT(i == 0)
  CERASTES_DECREASES(n - i)
  while (never_returns(i) + g(n) > 0) {
    i = i + 1;
  }
  return i;
}
