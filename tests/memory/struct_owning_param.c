// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: [[cerastes::owning]] on a struct S * is the borrow p == NULL || owns(*p); the body tests p before p->x and &p->x; callers pass &s and NULL
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

void sink(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(cerastes_live(q));

void set_x(CERASTES_OWNING struct S *p)
{
  if (p) {
    p->x = 1;
  }
}

void touch(CERASTES_OWNING struct S *p)
{
  if (p != NULL) {
    sink(&p->x);
  }
}

int use_owning(void)
{
  struct S s = {0, 3};
  set_x(&s);
  touch(&s);
  set_x(NULL);
  return s.y;
}
