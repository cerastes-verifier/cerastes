// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 7: a compound literal used as a value: a member read (C++ folds the constant, C reads the element), an argument, and (int){5} as its converted element
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
    CERASTES_ENSURES(r, r == s.y)
{
  return s.y;
}

int use(void)
    CERASTES_ENSURES(r, r == 4 + 9 + 5)
{
  int a = (struct S){3, 4}.y;
  int b = get((struct S){.x = 8, .y = 9});
  int c = (int){5};
  CERASTES_CHECK(a == 4 && b == 9 && c == 5);
  return a + b + c;
}
