// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/loop_written_atom_walk.c
// Pins: 0040 item 5: both cells hold 1, so the walk returns 1, not 2
#include <cerastes.h>
#include <stddef.h>

int walk(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 1 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 2)
{
  int y = 1;
  int *q = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (q == p || q == &y) && cerastes_owns(*q) && *q == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = (q == p) ? &y : p;
    i += 1;
  }
  return *q;
}
