// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69): *p->q in a loop invariant is accepted: every read of it is the guarded chain over the loop's places, framed at entry and at the back edge
#include <cerastes.h>

struct P {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  int x = 0;
  struct P s = {&x};
  struct P *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &x && *p->q == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    x += 1;
    i += 1;
  }
  return p == &s ? x : 0;
}
