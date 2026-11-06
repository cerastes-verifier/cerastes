// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2: two partnerless atoms on one cell (owns(*r) and live(r)): the output amount is split between them (divided by the number of atoms) and the caller holds the whole cell again; it writes through *p and sees g
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

static int g = 3;

int *both(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*r) && cerastes_live(r) && r == &g)
{
  return &g;
}

int main(void)
{
  int *p = both();
  *p = 6;
  CERASTES_CHECK(g == 6);
  return g - 6;
}
