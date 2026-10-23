// expect: error precondition @ L21
// mode: c23, c++20
// twin: tests/scalar/call_while_condition.c
// Pins: 0021: n may be INT_MAX, and the last evaluation of the condition then calls next(INT_MAX)
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (next(i) <= n) {
    i = i + 1;
  }
  return i;
}
