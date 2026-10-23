// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_for_step.c
// Pins: 0019 item 2: the continue skips the assignment, and the step reads x
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x;
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (; i < n; total = x) {
    i += 1;
    if (i == 3) {
      continue;
    }
    x = i;
  }
  return total;
}
