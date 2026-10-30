// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_written_atom.c
// Pins: 0040 items 3 and 5 (audit S1): the call in the loop consumes the cell: an iteration ends without owning *p
#include <cerastes.h>
#include <stddef.h>

void keep(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));

int use_owning(CERASTES_OWNING int *p, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == n)
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns(*p)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p != NULL) {
      keep(p);
    }
    i += 1;
  }
  return i;
}
