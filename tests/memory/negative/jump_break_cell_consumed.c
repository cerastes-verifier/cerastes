// expect: error lifetime @ L22
// mode: c23, c++20
// twin: tests/memory/jump_break_cell.c
// Pins: 0038 item 7: the callee kept the cell, so the break's scope exit fails at the break
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int stop_at(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r <= n && r <= 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && i <= 5)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = i;
    int *p = &x;
    if (*p == 5) {
      dispose(p);
      break;
    }
    i += 1;
  }
  return i;
}
