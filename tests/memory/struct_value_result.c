// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 items 4, 5: a struct result is one Viper result per member (res_m0, res_m1), return s reads every member, the caller's multi-target call stores each into its own struct
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S make(int a)
    CERASTES_ENSURES(r, r.x == a && r.y == 2)
{
  struct S s;
  s.x = a;
  s.y = 2;
  return s;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s = make(1);
  CERASTES_CHECK(s.x == 1);
  CERASTES_CHECK(s.y == 2);
  return s.x;
}
