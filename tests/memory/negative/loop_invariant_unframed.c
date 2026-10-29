// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/loop_cell_invariant.c
// Pins: 0038 item 6 (framing): the invariant reads x, whose object a callee kept before the loop: reported at the clause, before Silicon's own reason
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int invariant_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  dispose(&x);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
