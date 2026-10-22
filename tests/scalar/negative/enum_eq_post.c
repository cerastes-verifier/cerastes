// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_eq.c
// Pins: 0018 item 6(i): the branch for c == RED returns 2, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int is_red(enum color c)
    CERASTES_ENSURES(r, r == (c == RED))
{
  if (c == RED) {
    return 2;
  }
  return 0;
}

int not_blue(enum color c)
    CERASTES_ENSURES(r, r == (c != BLUE))
{
  if (c != BLUE) {
    return 1;
  }
  return 0;
}

int is_three(enum color c)
    CERASTES_ENSURES(r, r == (c == 3))
{
  if (c == 3) {
    return 1;
  }
  return 0;
}

int same(enum color a, enum color b)
    CERASTES_ENSURES(r, r == (a == b))
{
  if (a == b) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a = is_red(RED);
  int b = not_blue(GREEN);
  int c = same(BLUE, BLUE);
  int d = is_three(RED);
  return a + b + c + d;
}
