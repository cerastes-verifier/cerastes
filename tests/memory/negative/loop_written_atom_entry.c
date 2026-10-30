// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/loop_written_atom_param.c
// Pins: 0040 item 5: the function holds only live(p), so the loop enters without owning *p, which its invariant needs: reported at the atom
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_live(p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_live(p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
