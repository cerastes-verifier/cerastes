// expect: error overflow @ L15
// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/scalar/measure_expression.c
// Pins: 0013 item 7: the measure's arithmetic is an obligation at every loop head, not only the first
// The measure equals n - i; at i == 0 its arithmetic is fine, for i up to n the product i * 1000 overflows int.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 3000000)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + i * 1000 - i * 1000)
  while (i < n) {
    i += 1;
  }
  return i;
}
