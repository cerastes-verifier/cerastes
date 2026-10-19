// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/scalar/while_invariant.c
// Pins: 0012 item 8: an iteration must preserve the invariant
// The extra conjunct i <= 5 holds at entry and breaks when i goes from 5 to 6.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && i <= 5)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
