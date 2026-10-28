// expect: error uninitialized @ L20
// mode: c23, c++20
// twin: tests/memory/goto_past_cell.c
// Pins: 0032 item 4 and 0019 item 6: the label's return reads x, which the goto's path never wrote
#include <cerastes.h>

int skip_cell(int a)
    CERASTES_ENSURES(r, a > 5 || r == 6)
{
  int y = 0;
  if (a > 5) {
    goto done;
  }
  int x;
  int *q;
  q = &x;
  *q = 3;
  y = x;
done:
  return y + x;
}
