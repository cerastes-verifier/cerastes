// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9, 0042 items 4-7: must_init on a struct S *: the callee gets every leaf uninitialized and must write every leaf; the caller passes &s of a local without an initializer and reads both members after
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void init(struct S *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == 1 && p->y == 2)
{
  p->x = 1;
  p->y = 2;
}

int use_init(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s;
  init(&s);
  return s.x + s.y;
}
