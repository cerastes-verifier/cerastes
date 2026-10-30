// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/memory/cell_postfix_value.c
// Pins: 0039 item 2 and 0032 item 7: x++ as a value reads the cell before it has a value: uninitialized at the read of the snapshot
#include <cerastes.h>

int post_inc_uninitialized(void)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x;
  int *p = &x;
  int y = x++;
  *p = 0;
  return y;
}
