// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/loop_atom_automatic.c
// Pins: 0040 item 5 (audit B4, question 74): an atom on *q where q is a cell (its address is taken): the place is reached through a heap read, rejected until M4.5c
#include <cerastes.h>

void walk(int n)
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
