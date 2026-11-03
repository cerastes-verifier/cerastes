// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12: a written struct atom on a place reached through a load: owns(*p->q) in an invariant
#include <cerastes.h>
#include <stddef.h>

struct L {
  struct L *q;
  int x;
};

void walk(struct L *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q))
{
  struct L *q0 = p->q;
  (void)(q0 == p->q);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p->q == q0 && cerastes_owns(*p->q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->q->x = i;
    i += 1;
  }
}
