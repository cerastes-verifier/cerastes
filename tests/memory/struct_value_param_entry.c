// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 items 3, 5 (0012 item 2): the contract reads the parameter's entry value, so r == s.x + 1 holds although the body changed the copy, and the caller's own struct is unchanged
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int bump(struct S s)
    CERASTES_REQUIRES(s.x < 100)
    CERASTES_ENSURES(r, r == s.x + 1)
{
  s.x += 1;
  return s.x;
}

int use_bump(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct S s = {5, 1};
  int t = bump(s);
  CERASTES_CHECK(s.x == 5);
  return t;
}
