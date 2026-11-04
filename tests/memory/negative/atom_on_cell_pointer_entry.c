// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/atom_on_cell_pointer.c
// Pins: 0045 item 12: x's cell is given away before the loop, so owns(*q) does not hold on entry: permission at the clause
#include <cerastes.h>
void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void walk(int n) CERASTES_REQUIRES(0 <= n)
{
  int x = 0;
  int *q = &x;
  int **qq = &q;
  int i = 0;
  dispose(&x);
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*q))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **qq = i;
    i += 1;
  }
}
