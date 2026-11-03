// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: owns(p->x) holds one leaf and one liveness fraction: the function touches only p->x, the caller keeps s.y
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set_x(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(cerastes_owns(p->x) && p->x == 5)
{
  p->x = 5;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S s;
  s.x = 1;
  s.y = 2;
  set_x(&s);
  CERASTES_CHECK(s.x == 5 && s.y == 2);
  return s.y;
}
