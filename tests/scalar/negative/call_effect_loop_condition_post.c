// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/scalar/call_effect_loop_condition.c
// Pins: 0021: the result is i + c == 2 * n, not one less
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int f(int n)
    CERASTES_REQUIRES(1 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n - 1)
{
  int i = 1;
  int c;
  CERASTES_INVARIANT(1 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = pos(i)) < n) {
    i++;
  }
  return i + c;
}
