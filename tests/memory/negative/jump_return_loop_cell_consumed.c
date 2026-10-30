// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/jump_return_loop_cell.c
// Pins: 0038 item 7: the callee kept the cell, so the return's scope exit fails at the return
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int return_in_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= n && n <= 100)
  CERASTES_DECREASES(n)
  while (n > 0) {
    if (n == 50) {
      dispose(p);
      return 1;
    }
    n -= 1;
  }
  return *p;
}
