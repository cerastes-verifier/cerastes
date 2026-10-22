// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 10: <, <=, >, >= of two values of one enumeration, or of a value and one of its enumerators, mean the same in both modes
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int warm(enum color c)
    CERASTES_ENSURES(r, r == (c < BLUE ? 1 : 0))
{
  if (c < BLUE) {
    return 1;
  }
  return 0;
}

int ordered(enum color a, enum color b)
    CERASTES_ENSURES(r, r == (a <= b ? 1 : 0))
{
  if (a <= b) {
    return 1;
  }
  return 0;
}

int beyond_red(enum color c)
    CERASTES_ENSURES(r, r == (RED < c ? 1 : 0))
{
  if (RED < c) {
    return 1;
  }
  return 0;
}

int above_green(enum color c)
    CERASTES_ENSURES(r, r == (c > GREEN ? 1 : 0))
{
  if (c > GREEN) {
    return 1;
  }
  return 0;
}

int above_blue(enum color c)
    CERASTES_ENSURES(r, r == (c > BLUE ? 1 : 0))
{
  if (c > BLUE) {
    return 1;
  }
  return 0;
}

int green_below(int a)
    CERASTES_ENSURES(r, r == (GREEN < a ? 1 : 0))
{
  if (GREEN < a) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  enum color odd = (enum color)3;
  int a = warm(RED);
  int b = warm(odd);
  int c = ordered(GREEN, odd);
  int d = above_blue(odd);
  return a + b + c + d;
}
