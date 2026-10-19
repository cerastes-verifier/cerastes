// expect: error division-by-zero @ L14
// mode: c23, c++20
// twin: tests/scalar/loop_condition.c
// Pins: 0010 item 7 and 0012 item 8: n == 0 in the loop condition, at the while
// Reported once: after the first failing check n is assumed non-zero.
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 1000)
  CERASTES_DECREASES(1000 - i)
  while (i < 1000 / n) {
    i += 1;
  }
  return i;
}
