// expect: verified
// expect: native-ok
// Pins: 0044 item 6: C++ S s = S{1, 2}, S s = S() (every member zero), g(S{1, 2}), return S{} and a call result stored into a struct
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
  CERASTES_CHECK(a.y == 2 && b.x == 0 && z.y == 0);
  return a.x + b.x + c + z.y;
}
