// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69): p->q->x in a loop invariant is accepted: every read of it is the guarded chain over the loop's places, framed at entry and at the back edge
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  struct L t = {0, 0};
  struct L s = {&t, 0};
  struct L *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &t && p->q->x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    t.x += 1;
    i += 1;
  }
  return p == &s ? t.x : 0;
}
