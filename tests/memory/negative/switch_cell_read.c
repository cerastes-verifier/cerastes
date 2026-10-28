// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/switch_cell.c
// Pins: audit round 2 of M4.5b1a, 0032 item 4: the default case reads the cell without writing it: allocated by the dispatch, but uninitialized
#include <cerastes.h>

int pick(int code)
    CERASTES_ENSURES(r, code != 0 || r == 5)
{
  switch (code) {
    int x;
    int *p;
  case 0:
    p = &x;
    *p = 5;
    return x;
  default:
    p = &x;
    return x;
  }
}
