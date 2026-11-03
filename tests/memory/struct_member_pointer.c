// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 5, 9: &s.x is a pointer to a member: an int * whose owns(*p) is the one leaf; s.y is never touched and never read
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

int use_inc(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S s;
  s.x = 0;
  inc(&s.x);
  CERASTES_CHECK(s.x == 1);
  inc(&s.x);
  return s.x;
}
