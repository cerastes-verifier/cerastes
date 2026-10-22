// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_for_step.c
// Pins: 0019 item 2: x is assigned before the step on every path; the total is wrong
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, (n == 0 && r == 0) || (n >= 1 && r == n))
{
  int x;
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && ((i == 0 && total == 0) || (i >= 1 && total == i)))
  CERASTES_DECREASES(n - i)
  for (; i < n; total = x) {
    i += 1;
    x = i + 1;
    if (i == 3) {
      continue;
    }
  }
  return total;
}
