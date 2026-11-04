// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (audit round 2 S2): a loop that moves the global pointer gp between two local cells it owns whole, through written atoms (a swap of gp and r), the address tie and the value tie in the invariant; a by-name write of a global root is a move of its written atoms
#include <cerastes.h>
#include <stddef.h>

static int *gp;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == NULL)
{
  int x = 1;
  int y = 2;
  int *r = &y;
  gp = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(cerastes_owns(*gp) && cerastes_owns(*r) && ((gp == &x && r == &y) || (gp == &y && r == &x)))
  CERASTES_INVARIANT(((*gp == 1 && *r == 2) || (*gp == 2 && *r == 1)))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int *t = gp;
    gp = r;
    r = t;
    i = i + 1;
  }
  gp = NULL;
}
