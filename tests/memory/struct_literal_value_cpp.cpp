// expect: verified
// expect: native-ok
// Pins: 0044 item 7 (P67 H2): C++ (S){...} as a value: an assignment source, a folded member read, an argument
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(S s)
    CERASTES_ENSURES(r, r == s.y)
{
  return s.y;
}

int use()
    CERASTES_ENSURES(r, r == 4 + 9 + 1)
{
  S t = {0, 0};
  t = (S){1, 2};
  int a = (S){3, 4}.y;
  int b = get((S){8, 9});
  CERASTES_CHECK(t.x == 1 && a == 4 && b == 9);
  return a + b + t.x;
}
