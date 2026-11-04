// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (question 105): owns(p->q->x) as an automatic loop place (a member step, a load, a member)
#include <cerastes.h>
#include <stddef.h>

struct L {
  struct L *q;
  int x;
};

void bump_n(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x == 0 && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == n)
{
  struct L *q0 = p->q;
  (void)(q0 == p->q);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->q == q0 && p->q->x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->q->x += 1;
    i += 1;
  }
}
