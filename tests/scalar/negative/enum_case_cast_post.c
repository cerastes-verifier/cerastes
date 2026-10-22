// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_case_cast.c
// Pins: 0018 items 6(ii) and 9: the case (enum color)2 branch is BLUE and returns 3, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c == BLUE && r == 2) || (c != RED && c != BLUE && r == 0))
{
  switch (c) {
  case RED:
    return 1;
  case (enum color)2:
    return 3;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a = pick(RED);
  int b = pick(BLUE);
  int c = pick(GREEN);
  return a + b + c;
}
