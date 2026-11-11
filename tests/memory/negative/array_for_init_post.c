// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/array_for_init.c
// Pins: 0059 items 6 and 8 (audit round 2, item 5e): the loop ends with s == 3, not 2
// Pre-registered from audit round 2 of record 0059 (item 5e)
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  for (int a[2] = {1, 2}; s < 3; s++) {
    a[0] = s;
    if (a[0] != s) {
      return -1;
    }
  }
  return s;
}
