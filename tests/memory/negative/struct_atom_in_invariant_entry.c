// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/struct_atom_in_invariant.c
// Pins: 0045 item 12: the function owns only p->x, the atom needs all of *p: permission at the clause (entry)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void walk(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(p->x) && 0 <= n && n <= 10)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
