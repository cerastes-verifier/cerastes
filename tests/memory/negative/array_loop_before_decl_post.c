// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/array_loop_before_decl.c
// Pins: 0059 items 6 and 8 (audit round 2, item 5e): 1 + 3 + 3 is 7, not 8
// Pre-registered from audit round 2 of record 0059 (item 5e)
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 8)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  while (s < 3) {
    s++;
  }
  int a[3] = {1, 2, 3};
  return a[0] + a[2] + s;
}
