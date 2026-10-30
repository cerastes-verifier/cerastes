// expect: error unsequenced @ L18
// mode: c23, c++20
// twin: tests/memory/cell_conditional_target.c
// Pins: 0039 item 3 (audit S1): s = c ? q : &w may be q: the target's address reads *q and the value writes *s with no sequence point between (path-precise: different names, no compiler warns)
#include <cerastes.h>

int conditional_target(int c)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = 1;
  int y = 2;
  int z = 3;
  int w = 4;
  int *a = &x;
  int *b = &y;
  int *q = &z;
  int *s = c ? q : &w;
  *(*q > 0 ? a : b) = (*s)++;
  return x + y;
}
