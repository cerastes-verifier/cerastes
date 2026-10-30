// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 and the port plan: **pp = 1; *p = 2; check(**pp == 1) where the contract puts *pp on s, never on p (pairwise separation of the owned cells)
#include <cerastes.h>
#include <stddef.h>

void trap(int **pp, int *p, int *s)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*p) && cerastes_owns(*s) && *pp == s)
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*p) && cerastes_owns(*s))
{
  (void)s;
  **pp = 1;
  *p = 2;
  CERASTES_CHECK(**pp == 1);
}
