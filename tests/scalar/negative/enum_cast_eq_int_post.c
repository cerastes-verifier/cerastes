// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_cast_eq_int.c
// Pins: 0018 item 6(i): the branch for x == (int)(enum color)2 returns 2, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int is_two(int x)
    CERASTES_ENSURES(r, (x == 2 && r == 1) || (x != 2 && r == 0))
{
  if (x == (int)(enum color)2) {
    return 2;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = is_two(2);
  int b = is_two(a);
  int c = is_two(a + 1);
  return a + b + c;
}
