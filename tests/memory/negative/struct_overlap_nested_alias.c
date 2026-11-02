// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/struct_overlap_nested.c
// Pins: 0043 item 9: &t.t is the nested slot of *t: the two atoms share its leaves and the separation s != &t.t fails at the call
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
  g(&t, &t.t);  // <- here
  return t.z;
}
