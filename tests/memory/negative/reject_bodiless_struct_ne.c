// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B1, b_struct_ne.c): the same for a struct result's pointer member: r.q != p ? r.q == NULL : owns(*r.q)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B1: rejected unsupported_annotation at the clause line
#include <cerastes.h>
#include <stdlib.h>

struct P {
  int *q;
};

struct P g(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p))
    CERASTES_ENSURES(r, r.q != p ? r.q == NULL : cerastes_owns(*r.q));  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  struct P s = g(p);
  if (s.q != NULL) {
    free(p);
    free(p);
    return 2;
  }
  free(p);
  return 0;
}
