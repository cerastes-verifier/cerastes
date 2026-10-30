// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/loop_written_atom_param.c
// Pins: 0040 item 5: a callee in the body consumes the place the atom holds: an iteration ends without owning *p, reported at the body's closing brace
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(p);
    i += 1;
  }
}
