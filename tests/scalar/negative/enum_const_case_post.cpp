// expect: error postcondition @ L15
// twin: tests/scalar/enum_const_case.cpp
// Pins: 0018 items 6(ii) and 9: the case DEF branch returns 2, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

const color DEF = GREEN;

int pick(color c)
    CERASTES_ENSURES(r, (c == GREEN && r == 1) || (c != GREEN && r == 0))
{
  switch (c) {
  case DEF:
    return 2;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a = pick(GREEN);
  int b = pick(BLUE);
  return a + b;
}
