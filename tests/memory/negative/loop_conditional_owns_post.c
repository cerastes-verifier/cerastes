// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/loop_conditional_owns.c
// Pins: 0038 item 3 (audit round 2, B1): the cell holds 9 after the loop, not 10
#include <cerastes.h>

int guarded_loop(int *p, int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && r == 10)
{
  int x = 0;
  int *q = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && (i == 0 || x == i - 1))
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    *q = i;
    i += 1;
  }
  return x;
}
