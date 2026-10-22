// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 items 3 and 10: (enum color)3 is a value of the enumeration, above BLUE: it is not warm
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int warm(enum color c)
    CERASTES_ENSURES(r, (c < BLUE && r == 1) || (c >= BLUE && r == 0))
{
  if (c < BLUE) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  enum color odd = (enum color)3;
  int a = warm(RED);
  int b = warm(odd);
  int c = warm(BLUE);
  return a + b + c;
}
