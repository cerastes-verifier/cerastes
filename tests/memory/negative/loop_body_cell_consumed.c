// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/loop_body_cell.c
// Pins: 0038 items 2 and 7: the callee kept the body's cell, so its scope exit at the closing brace finds the object not whole
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int body_cells(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = 1;
    dispose(&x);
    i += 1;
  }
  return i;
}
