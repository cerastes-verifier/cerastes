// expect: error permission @ L21
// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/loop_written_atom_tie.c
// Pins: 0040 item 5 (S28 P8's gap): without p == q the invariant does not say where p ends: the back edge cannot show the place p = q is held, and the ensures cannot get the entry place back after the loop
#include <cerastes.h>
#include <stddef.h>

int tie(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *q = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_live(q) && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    p = q;
    i += 1;
  }
  return i;
}
