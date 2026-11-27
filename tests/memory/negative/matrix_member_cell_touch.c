// expect: error permission @ L18
// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/matrix_member_cell.c
// Pins: 0064 item 13: owns(p->n) frames the cell n alone, so a write to the matrix member p->m[i][j] is not covered: touch writes p->m[0][0] in a body, loop_touch writes p->m[1][i] in a loop whose invariant keeps owns(p->n)
// Pre-registered from record 0064 item 13 (an atom on a scalar cell does not own the member beside it), before any run: one permission at each write
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

void touch(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(cerastes_owns(p->n))
{
  p->m[0][0] = 1;  // <- here
}

int loop_touch(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(r, cerastes_owns(p->n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->n))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p->m[1][i] = i;  // <- here
    i = i + 1;
  }
  return 0;
}
