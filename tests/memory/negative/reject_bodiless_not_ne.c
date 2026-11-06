// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B1, b_not_ne.c): owns(*r) && !(r != p): the negation of != is == in disguise
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B1: rejected unsupported_annotation at the clause line
#include <cerastes.h>
#include <stdlib.h>

int *f(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_owns(*r) && !(r != p));  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int *q = f(p);
  (void)q;
  free(p);
  free(p);
  return 0;
}
