// expect: error termination @ L21
// mode: c23, c++20
// twin: tests/scalar/call_for_condition.c
// Pins: 0021: without the step the measure does not decrease
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int count_for(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  for (; next(i) <= n; ) {
  }
  return total;
}
