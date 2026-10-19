// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_loop.c
// Pins: 0013 item 2: the measure is negative at the start of the second to last iteration (n - i - 3 at i == n - 1)
// It does decrease, and the loop terminates, but the measure must be bounded below by 0 at every iteration but the last: Silicon compares consecutive iterations, so n - i - 2 (negative only at the start of the last iteration) verifies and n - i - 3 does not.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i - 3)
  while (i < n) {
    i += 1;
  }
  return i;
}
