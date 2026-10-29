// expect: error lifetime @ L25
// mode: c23, c++20
// twin: tests/memory/loop_body_cell.c
// Pins: 0038 item 7: a continue leaves the body's block: the exit of its cell is checked at the continue
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int body_cells(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = 1;
    int *p = &x;
    i += 1;
    if (i == 7) {
      total += *p;
      dispose(p);
      continue;
    }
    total += *p;
  }
  return total;
}
