// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 11 (0038 item 3): every leaf of a held struct place of an unassigned parameter is an automatic place: written in the loop, its value stated in the invariant
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void fill(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == n && p->y == 2 * n)
{
  int i = 0;
  p->x = 0;
  p->y = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->x == i && p->y == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x += 1;
    p->y += 2;
    i += 1;
  }
}
