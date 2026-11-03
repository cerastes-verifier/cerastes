// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 4 (0037 item 3): a result with a pointer member: WF of the result leaf, the no-op case split on every cell in scope, then a write through the returned pointer framed by the caller's own cells
#include <cerastes.h>

struct P {
  int *p;
  int v;
};

struct P pick(int *a, int *b, int c)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_owns(*b) && (r.p == a || r.p == b) && r.v == 0)
{
  struct P s;
  s.v = 0;
  s.p = c != 0 ? a : b;
  return s;
}

void use_pick(void)
{
  int c1 = 1;
  int c2 = 2;
  struct P t = pick(&c1, &c2, 1);
  int *x = t.p;
  *x = 9;
  CERASTES_CHECK(c1 == 9 || c2 == 9);
}
