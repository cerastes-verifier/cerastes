// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/scalar/two_loops.c
// Pins: 0012 item 8: the second loop's invariant is not preserved
// total grows by 2 per iteration while the invariant says by 1.
#include <cerastes.h>

int count_both(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1000)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    total += 1;
  }
  int j = 0;
  CERASTES_INVARIANT(0 <= j && j <= n && total == n + j)
  CERASTES_DECREASES(n - j)
  while (j < n) {
    j += 1;
    total += 2;
  }
  return total;
}
