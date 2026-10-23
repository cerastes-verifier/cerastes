// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/scalar/while_incr_condition.c
// Pins: 0020 (until M4.3b): an effect with a call in a loop condition is rejected
#include <cerastes.h>

int pos(int v)
    CERASTES_REQUIRES(v > 0)
{
  return v;
}

int f(int n)
    CERASTES_REQUIRES(1 <= n && n < 1000)
{
  int i = 1;
  int c = 0;
  CERASTES_INVARIANT(1 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = pos(i)) < n) {
    i++;
  }
  return i + c;
}
