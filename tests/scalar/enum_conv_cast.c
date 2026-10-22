// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 items 3 and 5: (enum color)x is checked against the values of the enumeration, 0 to 3; 3 is a value though no enumerator is 3
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color from_int(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
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
