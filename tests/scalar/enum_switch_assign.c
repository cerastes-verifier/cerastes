// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1 and 0019 item 11: a switch on an assignment of enumeration type is a switch on an enumeration value
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == (n == 3 ? 3 : 10 * (n + 1)))
{
  enum color c = RED;
  switch (c = (enum color)n) {
  case RED:
    return 10 + (int)c;
  case GREEN:
    return 19 + (int)c;
  case BLUE:
    return 28 + (int)c;
  }
  return (int)c;
}
