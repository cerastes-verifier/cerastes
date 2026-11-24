// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 4: a call in a function's decreases is rejected (M4.5f1: measures), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int ms(int x)
{
  return x;
}

int count(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_DECREASES(ms(n))
{
  if (n == 0) {
    return 0;
  }
  return count(n - 1);
}
