// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B1, b_ne_guard2.c): a bodiless contract that ties the result to a formal through != in a guard (r != p ? r == NULL : owns(*r)): no == appears, but the result is p where it is not NULL, so the double free after the call would verify: rejected at the clause
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B1: rejected unsupported_annotation (2a message) at the clause line (a guess: the declaration's first line is the other candidate)
#include <cerastes.h>
#include <stdlib.h>

int *f(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p))
    CERASTES_ENSURES(r, r != p ? r == NULL : cerastes_owns(*r));  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int *q = f(p);
  if (q != NULL) {
    free(p);
    free(p);
    return 2;
  }
  free(p);
  return 0;
}
