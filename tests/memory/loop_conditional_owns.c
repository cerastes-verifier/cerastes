// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (audit round 2, B1): a parameter with c == 0 || owns(*p) in requires and ensures, and a cell written through a pointer in a loop: the conditional atom's place is guarded, the cell's value is in the invariant
#include <cerastes.h>

int guarded_loop(int *p, int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && r == 9)
{
  if (c != 0) {
    (void)p;
  }
  int x = 0;
  int *q = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && (i == 0 || x == i - 1))
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    *q = i;
    i += 1;
  }
  return x;
}
