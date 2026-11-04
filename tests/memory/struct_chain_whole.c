// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3: owns(*p->q) (the whole struct behind a member) and live(p->q): the callee writes p->q->x, the other compares the loaded pointer
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void zero(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->x == 0)
{
  p->q->x = 0;
}

int same(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_live(p->q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_live(p->q) && r == 1)
{
  struct L *a = p->q;
  return a == p->q;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct L t = {0, 5};
  struct L s = {&t, 1};
  zero(&s);
  CERASTES_CHECK(t.x == 0);
  return same(&s);
}
