// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9 (overlap by slots, never by the atom's pointee type): an owns atom on a struct S * and one on an int * name different cells when the int * points into another object
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *s, int *q)
    CERASTES_REQUIRES(cerastes_owns(*s) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*s) && cerastes_owns(*q) && s->x == 1 && *q == 2 && s->y == cerastes_old(s->y))
{
  s->x = 1;
  *q = 2;
}

int use_f(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S s = {5, 6};
  struct S t = {7, 8};
  f(&s, &t.x);
  CERASTES_CHECK(s.x == 1 && s.y == 6 && t.x == 2 && t.y == 8);
  return t.x;
}
