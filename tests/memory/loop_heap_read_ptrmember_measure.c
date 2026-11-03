// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69): *p->q in a loop measure is accepted, with the same framing
#include <cerastes.h>

struct P {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  int x = n;
  struct P s = {&x};
  struct P *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &x && x == n)
  CERASTES_DECREASES(*p->q - i)
  while (i < n) {
    i += 1;
  }
  return p == &s ? x : 0;
}
