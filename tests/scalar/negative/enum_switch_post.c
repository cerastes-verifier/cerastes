// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_switch.c
// Pins: 0018 item 6(ii): the GREEN case returns 21, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int weight(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 10) || (c == GREEN && r == 20) || (c == BLUE && r == 30) || (c != RED && c != GREEN && c != BLUE && r == 0))
{
  switch (c) {
  case RED:
    return 10;
  case GREEN:
    return 21;
  case BLUE:
    return 30;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 60)
{
  int a = weight(RED);
  int b = weight(GREEN);
  int c = weight(BLUE);
  int d = weight((enum color)3);
  return a + b + c + d;
}
