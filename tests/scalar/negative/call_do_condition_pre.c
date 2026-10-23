// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/scalar/call_do_condition.c
// Pins: 0021: n may be 101, and the last evaluation of the condition then calls step(100)
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int test(int n)
    CERASTES_REQUIRES(n <= 101)
    CERASTES_ENSURES(r, r == (n > 1 ? n - 1 : 1))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && (i == 0 || i + 2 <= n))
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (step(i) < n);
  return i;
}
