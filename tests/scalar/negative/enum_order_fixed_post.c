// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/enum_order_fixed.c
// Pins: 0019 item 10: unsigned long base: b1 > b0, so the contract's answer for ordered(B1, B0) is wrong when it says 1
#include <cerastes.h>

enum big : unsigned long { B0, B1, B2 };

int ordered(enum big a, enum big b)
    CERASTES_ENSURES(r, (a <= b && r == 1) || (a > b && r == 0))
{
  if (a < b) {
    return 1;
  }
  return 0;
}
