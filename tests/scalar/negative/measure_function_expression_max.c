// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/scalar/measure_function_expression.c
// Pins: 0013 item 7: the caller's requires n <= 1001 lets 1001 through, which the callee's n <= 1000 refuses (the last function of the positive)
#include <cerastes.h>

int down(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1000)
    CERASTES_DECREASES(n * 1000)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return down(n - 1);
}

// The largest n the requires of `down` allows is passed on: a stronger upper bound fails here.
int down_max(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1001)
    CERASTES_ENSURES(r, r == 0)
{
  return down(n);
}
