// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/scalar/enum_switch_values.c
// Pins: 0019 item 11: a switch on (int)c with case 4 and case -1: they are never taken, c == 3 reaches the default
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int by_int(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c != RED && r == 0))
{
  switch ((int)c) {
  case 0:
    return 1;
  case 4:
    return 2;
  case -1:
    return 3;
  default:
    return 7;
  }
}
