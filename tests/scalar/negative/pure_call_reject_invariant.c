// expect: rejected cerastes-call_in_annotation @ L17
// mode: c23, c++20
// twin: tests/scalar/measure_loop.c
// Pins: 0062 item 4 (M4.5e1d Step 0 staging pin): a call of a pure function in a loop invariant is call_in_annotation until the part that lifts it; retired by Flips A with a positive of the same program
// Pre-registered from record 0062 item 4, before the code: rejected today, at the call
#include <cerastes.h>

CERASTES_PURE bool small(int x)
{
  return -100 < x && x < 100;
}

int f(void)
    CERASTES_ENSURES(r, r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && small(i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  return i;
}
