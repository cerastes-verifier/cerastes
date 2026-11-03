// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12 (flip of reject_atom_on_cell_pointer.c): an atom on *q where q is a cell (its address is taken): the atom's object is the chained read of q through the cell, and the body writes it through qq
#include <cerastes.h>

void walk(int n) CERASTES_REQUIRES(0 <= n)
{
  int x = 0;
  int *q = &x;
  int **qq = &q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **qq = i;
    i += 1;
  }
}
