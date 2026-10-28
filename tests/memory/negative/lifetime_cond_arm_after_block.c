// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/cond_arm_pointer.c
// Pins: audit round 2 of M4.5b1a, 0032 items 6, 7: q = c ? p : &y with p dangling uses p in the arm: a lifetime error
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 0 : 1))
{
  int x = 1;
  int y = 2;
  int *p = &x;
  {
    int z = 3;
    p = &z;
  }
  int *q = c != 0 ? p : &y;
  if (q == &y) {
    return 1;
  }
  return 0;
}
