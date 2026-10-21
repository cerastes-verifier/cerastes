// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/continue_no_step.c
// Pins: 0014 item 4: a continue that skips the increment does not decrease the measure
// The while loop has its increment in the body, so the continue at i == 3 repeats the iteration.
#include <cerastes.h>

int count_not_three(int n)
    CERASTES_REQUIRES(0 <= n)
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(i <= n && 0 <= c && c <= i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 3) {
      continue;
    }
    c += 1;
    i += 1;
  }
  return c;
}
