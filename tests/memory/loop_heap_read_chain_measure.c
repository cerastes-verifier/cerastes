// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69): p->q->x in a loop measure is accepted, with the same framing
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  struct L t = {0, n};
  struct L s = {&t, 0};
  struct L *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &t && t.x == n)
  CERASTES_DECREASES(p->q->x - i)
  while (i < n) {
    i += 1;
  }
  return p == &s ? t.x : 0;
}
