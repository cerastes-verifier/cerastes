// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 5, 9 (M4's acceptance, a struct field update): owns(*c) over a struct pointer parameter, c->n += c->step, old(c->n) in the ensures; the caller passes &k of a struct local
#include <cerastes.h>

struct counter {
  int n;
  int step;
};

void tick(struct counter *c)
    CERASTES_REQUIRES(cerastes_owns(*c) && 0 <= c->n && c->n <= 100 && 0 <= c->step && c->step <= 10)
    CERASTES_ENSURES(cerastes_owns(*c) && c->n == cerastes_old(c->n) + cerastes_old(c->step) && c->step == cerastes_old(c->step))
{
  c->n += c->step;
}

int use_tick(void)
    CERASTES_ENSURES(r, r == 14)
{
  struct counter k;
  k.n = 4;
  k.step = 10;
  tick(&k);
  CERASTES_CHECK(k.n == 14 && k.step == 10);
  return k.n;
}
