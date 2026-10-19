// expect: error invariant-entry @ L13
// mode: c23, c++20
// twin: tests/scalar/while_invariant.c
// Pins: 0012 item 8: the invariant must hold when the loop is first reached
// With i starting at 1 the invariant fails on entry when n == 0.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 1;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
