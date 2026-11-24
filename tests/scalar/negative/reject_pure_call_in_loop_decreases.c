// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 4: a call in a loop's decreases is rejected (M4.5f1: measures), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int ms(int x)
{
  return x;
}

int count(void)
    CERASTES_ENSURES(r, r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(ms(3 - i))
  while (i < 3) {
    i = i + 1;
  }
  return i;
}
