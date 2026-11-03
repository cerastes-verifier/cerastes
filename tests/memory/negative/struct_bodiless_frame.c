// expect: error permission @ L16
// (no second error: owner, 2026-11-02, question 94 — after a call to a function whose contract fails framing the caller's state is not checked further, 0037 item 7)
// mode: c23, c++20
// twin: tests/memory/struct_owns_param.c
// Pins: 0043 audit S1: the bodiless contract reads *q under no guard that frames it: permission at the clause; the check at L22 is not reached (0037 item 7's dummy chain makes the call's ensures false on that path; question 94)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

int g(struct S *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && (q == NULL || cerastes_owns(*q)))
    CERASTES_ENSURES(r, cerastes_owns(*p) && (q == NULL || cerastes_owns(*q)) && (cerastes_old(p->x) == cerastes_old(p->y) || *q > 0));  // <- here

int use(void)
{
  struct S s = {1, 2};
  int r = g(&s, NULL);
  CERASTES_CHECK(0 == 1);  // <- here
  return r;
}
