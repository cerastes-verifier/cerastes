// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0043 item 9, 0042 item 8: --strict-init: a struct local without an initializer is marked, its address goes to a must_init parameter, a struct local with an initializer needs no marker
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
  CERASTES_UNINIT struct S s;
  init(&s);
  return s.x + s.y;
}

int plain(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {1, 2};
  return s.x + s.y;
}
