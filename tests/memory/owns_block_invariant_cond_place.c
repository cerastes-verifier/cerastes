// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 4 ("Forms": a written site through a load under a guard) and audit round 2 (Parts C and D), probe c1: the site *pp is held only when c != 0, in the contract and in the invariant; the loop does nothing with it
// Pre-registered from record 0054 item 4 and audit probe c1 (exit 3 "if.failed" when written): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp, int c)
    CERASTES_REQUIRES(c == 0 || (cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp))))
    CERASTES_ENSURES(c == 0 || (cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp))))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(c == 0 || (*pp == NULL || cerastes_owns_block(*pp)))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (c != 0 && *pp != NULL) {
      **pp = i;
    }
    i += 1;
  }
}
