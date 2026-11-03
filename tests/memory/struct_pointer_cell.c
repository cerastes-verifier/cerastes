// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 8, 0040 item 2: a struct S ** parameter: owns(*pp) is a pointer cell; the callee stores q, the caller learns *pp == q and reads through it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set(struct S **pp, struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_live(q))
{
  *pp = q;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct S a = {1, 2};
  struct S b = {3, 4};
  struct S *p = &a;
  set(&p, &b);
  return p->y;
}
