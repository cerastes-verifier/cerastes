// expect: error conversion @ L9
// mode: c23, c++20
// twin: tests/scalar/enum_const.c
// Pins: 0018 item 1 and 0017 item 4: the constant's initializer (enum color)4 is outside 0 to 3, checked in the prologue
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

const enum color DEF = (enum color)4;

int is_default(enum color c)
    CERASTES_ENSURES(r, r == (c == DEF))
{
  if (c == DEF) {
    return 1;
  }
  return 0;
}

enum color default_color(void)
    CERASTES_ENSURES(r, r == DEF)
{
  return DEF;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = is_default(default_color());
  int b = is_default(GREEN);
  return a + b;
}

int from_const(void)
    CERASTES_ENSURES(r, r == 1)
{
  int i = DEF;
  return i;
}
