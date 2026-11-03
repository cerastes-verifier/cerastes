// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 7: t = s, struct S u = s, struct S v = *p, *dst = *src and the exact self-copy *p = *p, w.t = s into a nested member and x = w.t out of one: every source leaf is read first (permission and flag), then every destination leaf is written, flag set
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void copy(struct S *dst, struct S *src)
    CERASTES_REQUIRES(cerastes_owns(*dst) && cerastes_owns(*src))
    CERASTES_ENSURES(cerastes_owns(*dst) && cerastes_owns(*src) && dst->x == src->x && dst->y == src->y && src->x == cerastes_old(src->x) && src->y == cerastes_old(src->y))
{
  *dst = *src;
}

void self_copy(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->x) && p->y == cerastes_old(p->y))
{
  *p = *p;
}

int use_copy(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {1, 2};
  struct S t;
  t = s;
  CERASTES_CHECK(t.x == 1 && t.y == 2);
  struct S u = s;
  struct S *p = &u;
  struct S v = *p;
  copy(&t, &v);
  self_copy(&u);
  CERASTES_CHECK(u.x == 1 && u.y == 2 && v.x == 1);
  struct T w;
  w.t = s;
  w.z = 0;
  CERASTES_CHECK(w.t.x == 1 && w.t.y == 2);
  struct S x = w.t;
  return x.x + v.y;
}
