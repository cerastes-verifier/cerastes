// expect: error division-by-zero @ L18
// mode: c23, c++20
// twin: tests/scalar/do_condition_div.c
// Pins: 0014 item 5: the division in the condition is checked after the body, with the body's effect
// Without d = 1 the divisor is still 0 when the condition is evaluated.
#include <cerastes.h>

int once_with_division(int n)
    CERASTES_REQUIRES(n <= 5)
    CERASTES_ENSURES(r, r == 1)
{
  int d = 0;
  int count = 0;
  CERASTES_INVARIANT(count == 0)
  CERASTES_DECREASES(1 - count)
  do {
    count += 1;
  } while (n / d > 5);
  return count;
}
