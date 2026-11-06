// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B1, b_cond_eq.c): the result equated with a formal through a pointer ?: operand ((c ? r : r) == p)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B1: rejected unsupported_annotation at the clause line
#include <cerastes.h>
#include <stdlib.h>

int *f(int *p, int c)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_owns(*r) && (c ? r : r) == p);  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int *q = f(p, 1);
  (void)q;
  free(p);
  free(p);
  return 0;
}
