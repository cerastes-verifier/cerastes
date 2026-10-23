// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11: case 1 is GREEN, so the second case returns 2, not 9
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int by_value(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c == GREEN && r == 2) || (c == BLUE && r == 3) || (c == 3 && r == 0))
{
  switch (c) {
  case RED:
    return 1;
  case 1:
    return 9;
  case (enum color)2:
    return 3;
  }
  return 0;
}
