// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/enum_switch_assign.c
// Pins: 0020 item 1: case GREEN is taken for n == 1
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
    return 20 + (int)c;
  case BLUE:
    return 28 + (int)c;
  }
  return (int)c;
}
