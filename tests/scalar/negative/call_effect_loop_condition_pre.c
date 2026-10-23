// expect: error precondition @ L22
// mode: c23, c++20
// twin: tests/scalar/call_effect_loop_condition.c
// Pins: 0021: i starts at 0, so the first evaluation calls pos(0)
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(int n)
    CERASTES_REQUIRES(1 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = pos(i)) < n) {
    i++;
  }
  return i + c;
}
