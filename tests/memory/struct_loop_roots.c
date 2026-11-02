// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 11: two struct roots in one loop: four leaf places, two liveness rows (one per root block), none per leaf
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void both(struct S *p, struct S *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && p->x == n && q->y == n)
{
  int i = 0;
  p->x = 0;
  q->y = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->x == i && q->y == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->x += 1;
    q->y += 1;
    i += 1;
  }
}

int use_both(void)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  both(&s, &t, 3);
  return s.x + t.y;
}
