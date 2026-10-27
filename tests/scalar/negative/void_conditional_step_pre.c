// expect: error precondition @ L32
// mode: c23, c++20
// twin: tests/scalar/void_conditional_for.c
// Pins: 0025 item 3: the precondition of the step's call pos(i) is checked in the step, where i is 0 after the first iteration when c holds
#include <cerastes.h>

void pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  (void)x;
}

void neg(int y)
    CERASTES_REQUIRES(y < 0)
{
  (void)y;
}

int id(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

int steps(int c, int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; (c ? pos(i) : (void)0), i++) {
  }
  return i;
}
