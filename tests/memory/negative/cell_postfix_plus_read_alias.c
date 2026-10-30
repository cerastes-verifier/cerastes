// expect: error unsequenced @ L13
// mode: c23, c++20
// twin: tests/memory/cell_postfix_plus_read.c
// Pins: 0039 item 3: p = c ? &x : &y may be &x: x++ + *p modifies and reads one object on that path, so x != p fails
#include <cerastes.h>

int sum_after(int c)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = 1;
  int y = 10;
  int *p = c ? &x : &y;
  int s = x++ + *p;
  return s;
}
