// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: a struct T atom beside a struct S atom: g(&t, &u.t) names two different objects, the separation is on the slot of the nested struct
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void g(struct T *t, struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*t) && cerastes_owns(*s))
    CERASTES_ENSURES(cerastes_owns(*t) && cerastes_owns(*s) && t->z == s->x && s->x == cerastes_old(s->x))
{
  t->z = s->x;
}

int use_g(void)
{
  struct T t = {{1, 2}, 3};
  struct T u = {{4, 5}, 6};
  g(&t, &u.t);
  CERASTES_CHECK(t.z == 4);
  return t.z;
}
