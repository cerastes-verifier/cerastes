// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 7 (audit round 2 S7): the requires atom of q is taken while *c == 0 on entry and the body makes the guard false; the moved **pp returns q's object at q's amount, which needs the guard in the ENTRY state (old(g_i)), else it would fall back to the partner's amount
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *q, int *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*pp) && cerastes_owns(**pp) && (*c != 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_owns(*c) && cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(**pp))
{
  if (*c == 0) {
    *pp = q;
    *c = 1;
  }
}
