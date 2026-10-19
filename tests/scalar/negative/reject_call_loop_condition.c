// expect: rejected cerastes-unsupported_expression @ L19
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0012 item 6: a call in a loop condition is rejected
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  return x;
}

int test(int n)
    CERASTES_REQUIRES(n > 0 && n < 100)
{
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= 100)
  CERASTES_DECREASES(100 - i)
  while (i < pos(n)) {
    i += 1;
  }
  return i;
}
