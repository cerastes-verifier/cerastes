// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_order_negative.c
// Pins: 0019 item 10: the arms of cmp are swapped
#include <cerastes.h>

enum sign { NEG = -1, ZERO, POS };

int cmp(enum sign a, enum sign b)
    CERASTES_ENSURES(r, (a < b && r == -1) || (a == b && r == 0) || (a > b && r == 1))
{
  if (a < b) {
    return 1;
  }
  if (a > b) {
    return 1;
  }
  return 0;
}
