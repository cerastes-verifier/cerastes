// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (audit B2): old(**pp) read under the entry guard of its framing disjunct: one old over the whole chain (the guard *c != 0 is the entry value, *c is 0 on return)
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && cerastes_owns(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_REQUIRES(*c == 0 || **pp < 100)
    CERASTES_ENSURES(cerastes_owns(*c) && *c == 0 && cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (cerastes_old(*c) == 0 || cerastes_owns(**pp)))
    CERASTES_ENSURES(cerastes_old(*c) == 0 || **pp == cerastes_old(**pp) + 1)
{
  if (*c != 0) {
    **pp += 1;
  }
  *c = 0;
}
