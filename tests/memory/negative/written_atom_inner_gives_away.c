// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/written_atom_inner.c
// Pins: 0045 item 12: an iteration gives the atom's object away: the back-edge row fails (permission at the body's closing brace)
#include <cerastes.h>
#include <stddef.h>
void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int walk2(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 1;
  int *p = &x;
  int **pp = &p; if (pp) { }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && pp == &p && *pp == &x && cerastes_owns(**pp))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(&x);
    i += 1;
  }  // <- here
  return 0;
}
