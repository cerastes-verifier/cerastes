// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/jump_goto_cell.c
// Pins: 0038 item 7: the callee kept the cell, so the goto's scope exit fails at the goto
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int goto_cell(int a)
    CERASTES_REQUIRES(-100 <= a && a <= 100)
    CERASTES_ENSURES(r, (a > 5 && r == 0) || (a <= 5 && r == a))
{
  int total = 0;
  {
    int x = a;
    int *p = &x;
    if (*p > 5) {
      dispose(p);
      goto done;
    }
    total = x;
  }
done:
  return total;
}
