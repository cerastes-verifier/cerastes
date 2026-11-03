// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/written_atom_leaf.c
// Pins: 0045 item 12: the function owns nothing of *s.q: the atom's entry row fails (permission at the clause)
#include <cerastes.h>
#include <stddef.h>

struct P {
  int *q;
};

void walk(struct P s, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*s.q))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
