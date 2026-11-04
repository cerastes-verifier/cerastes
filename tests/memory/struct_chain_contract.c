// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3: owns(*p) && owns(p->q->x) (a member step, a load, a member): the callee increments through the chain, the caller reads the pointed-to struct afterwards
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void inc(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == cerastes_old(p->q->x) + 1)
{
  p->q->x += 1;
}

int get(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == cerastes_old(p->q->x) && r == p->q->x)
{
  return p->q->x;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct L t = {0, 3};
  struct L s = {&t, 0};
  inc(&s);
  CERASTES_CHECK(t.x == 4);
  return get(&s);
}
