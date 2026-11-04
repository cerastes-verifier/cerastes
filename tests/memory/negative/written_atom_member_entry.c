// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/written_atom_member.c
// Pins: 0045 item 12: the function does not own *p->q: the atom's entry row fails (permission at the clause)
#include <cerastes.h>
#include <stddef.h>

struct L {
  struct L *q;
  int x;
};

void walk(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p->q))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
