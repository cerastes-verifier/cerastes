// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 5 (audit B1): two bodiless owns(**) atoms, both tied: the pair-distinctness check and the partner rows pass
#include <cerastes.h>
#include <stddef.h>

void two(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns(**pp) && cerastes_owns(**qq))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(*qq) && *qq == cerastes_old(*qq) && cerastes_owns(**pp) && cerastes_owns(**qq));

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  int *q = &y;
  two(&p, &q);
  CERASTES_CHECK(p == &x && q == &y);
  return 0;
}
