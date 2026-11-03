// expect: error check @ L30
// twin: tests/memory/struct_value_cpp.cpp
// Pins: 0044 item 6: S b = S() zeroes every member, so check(b.x == 1) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

S zero()
    CERASTES_ENSURES(r, r.x == 0 && r.y == 0)
{
  return S{};
}

int use()
    CERASTES_ENSURES(r, r == 2)
{
  S a = S{1, 2};
  S b = S();
  int c = get(S{1, 2});
  S z = zero();
  CERASTES_CHECK(a.y == 2 && b.x == 1 && z.y == 0);  // <- here
  return a.x + b.x + c + z.y;
}
