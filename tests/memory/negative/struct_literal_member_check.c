// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/struct_literal_member.c
// Pins: 0044 item 7: (struct S){3, 4}.y is 4: check(a == 3) fails
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
  CERASTES_CHECK(a == 3 && b == 9 && c == 5);  // <- here
  return a + b + c;
}
