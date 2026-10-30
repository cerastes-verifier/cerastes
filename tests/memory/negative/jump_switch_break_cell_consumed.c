// expect: error lifetime @ L27
// mode: c23, c++20
// twin: tests/memory/jump_switch_break_cell.c
// Pins: 0038 item 7: the callee kept the case's cell, so the break out of its block fails at the break
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int switch_cell(int code)
    CERASTES_ENSURES(r, (code == 0 && r == 5) || (code == 1 && r == 3) || (code != 0 && code != 1 && r == 1))
{
  int total = 0;
  switch (code) {
    int x;
    int *p;
  case 0:
    p = &x;
    *p = 5;
    total = x;
    break;
  case 1: {
    int y = 3;
    int *q = &y;
    total = *q;
    dispose(q);
    break;
  }
  default:
    total = 1;
  }
  return total;
}
