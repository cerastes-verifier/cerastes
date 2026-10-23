// expect: error termination @ L20
// mode: c23, c++20
// twin: tests/scalar/call_do_condition.c
// Pins: 0021: a measure that does not depend on the iteration does not decrease
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x + 1 && 0 < r && r <= 100)
{
  return x + 1;
}

int test(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == (n > 1 ? n - 1 : 1))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && (i == 0 || i + 2 <= n))
  CERASTES_DECREASES(n)
  do {
    i += 1;
  } while (step(i) < n);
  return i;
}
