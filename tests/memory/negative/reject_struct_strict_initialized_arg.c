// expect: rejected cerastes-uninit_ref_initialized @ L24
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/struct_strict_init.c
// Pins: 0043 item 9, 0042 item 8: --strict-init: the argument of a must_init parameter must be &s of an uninit struct local, not of an initialized one
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
{
  struct S s = {5, 6};
  init(&s);  // <- here
  return s.x;
}
