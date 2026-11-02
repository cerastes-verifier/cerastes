// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0043 item 6 (C++20): S s{...}, S s{} and S s = {} (a `struct S` aggregate with only implicit trivial special members), designators in order
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int braces()
    CERASTES_ENSURES(r, r == 3)
{
  S s{1, 2};
  return s.x + s.y;
}

int value_init()
    CERASTES_ENSURES(r, r == 0)
{
  S s{};
  return s.x + s.y;
}

int equals_empty()
    CERASTES_ENSURES(r, r == 0)
{
  S s = {};
  CERASTES_CHECK(s.x == 0 && s.y == 0);
  return s.x;
}

int designated_cpp()
    CERASTES_ENSURES(r, r == 7)
{
  S s{.x = 3, .y = 4};
  return s.x + s.y;
}
