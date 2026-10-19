// expect: error overflow @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_expression.c
// Pins: 0013 item 7: n * 100000 overflows int for n up to 100000: overflow at the measure
// The measure's own arithmetic is an obligation at the loop head; a termination error is not what is reported.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100000)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n * 100000 - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
