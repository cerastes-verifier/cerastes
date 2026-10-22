// expect: error conversion @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_conv_cast.c
// Pins: 0018 item 5: x may be 4, which is not one of the values 0 to 3 of enum color
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color from_int(int x)
    CERASTES_REQUIRES(0 <= x && x <= 4)
    CERASTES_ENSURES(r, (int)r == x)
{
  enum color c = (enum color)x;
  return c;
}

int round_trip(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, r == x)
{
  enum color c = from_int(x);
  return (int)c;
}

int edges(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a = round_trip(0);
  int b = round_trip(3);
  return a + b;
}
