// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_for.c
// Pins: 0013 item 2: the increment goes the wrong way, so the measure grows from one iteration to the next
// The for loop's increment is part of the iteration the measure is compared over.
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == 0)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i += 0) {
    c += 0;
  }
  return c;
}
